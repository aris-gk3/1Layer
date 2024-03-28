#include "header.h"
#include <iostream>

//******************************* New Build  *************************************************************
// NOTES
// 1. Canonical Dataflow forms
// 2. Add extra variables as function I/O for info on state and addresses
// 3. Mechanism with which some function run more frequently than others in the same dataflow region.
// 4. Indices in Dataflow regions pass from process to process.
// 5. "Conditional" executions of load_IfMap(), load_WtMap() are not yet implemented.
// Possible Changes
// 1. Join/Merge Loop_Tof_step, Loop_Toy_step, Loop_Tox_step, Nif_Loop, Nky_Loop, Nkx_Loop.
// 2. IfMap, WtMap have different order in x-y coordinates.
// 3. Maybe remove IfMap_x_Address, east_pad, west_pad from load_IfMap()

// (Manual) Loop Interchange
void CNN_Layer_top(px_data_t IfMap[NIF][NIX-2*ZERO_PAD][NIY-2*ZERO_PAD],
	wt_data_t WtMap[NIF][NOF][NKX][NKY],
	px_data_t OfMap[NOF][NOX][NOY]){
	px_data_t InBuf[POY][WRD_INBUF][POX];
	//px_data_t InBuf[POY][WRD_INBUF][POX] = {0};
#pragma HLS ARRAY_PARTITION variable=InBuf complete dim=1
#pragma HLS ARRAY_PARTITION variable=InBuf complete dim=3
	wt_data_t WtBuf[WRD_WTBUF][POF];
#pragma HLS ARRAY_PARTITION variable=WtBuf complete dim=2
	px_data_t OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX];
#pragma HLS ARRAY_PARTITION variable=OutBuf complete dim=1
#pragma HLS ARRAY_PARTITION variable=OutBuf complete dim=3
	// I don't have loop for Nox, because I assume we load whole row.
	// NOF, NOY should be variable parameters and not constants.
	Loop_Noy: for(int Noy_step=0;Noy_step<my_ceil(NOY,TOY);Noy_step++){ // Tripcount=3
#pragma HLS PIPELINE off
		load_IfMap(IfMap, 0, 1, 1, InBuf);
		Loop_Nof: for(int Nof_step=0;Nof_step<my_ceil(NOF,TOF);Nof_step++){ // Tripcount=2
			load_WtMap(WtMap, WtBuf);
			compute_Maps(InBuf, WtBuf, OutBuf);
			store_Maps(OutBuf, OfMap);
		}
	}
}

// 1st Level of Modules

// Alternative to IF2BUF()
// For Now I use old implementation
void load_IfMap(px_data_t IfMap[NIF][NIX-2*ZERO_PAD][NIY-2*ZERO_PAD],
	int IfMap_x_Address, data_bool east_pad, data_bool west_pad,
	px_data_t InBuf[POY][WRD_INBUF][POX]){
	static int Noy_i = 0;
	int IfMap_y_Tile_Address;
	int Buf_bank,Buf_word,Buf_Wrd_base=0,IfMap_y_Address;
	int InBuf_banks_precompute = 0, InBuf_rows_precompute = 0, InBuf_rows_count = 0;
	data_bool north_pad;
	data_bool south_pad;

	north_pad = (Noy_i==0);
	south_pad = (Noy_i==my_ceil(NOY,TOY)-1);
	if(north_pad){
		IfMap_y_Tile_Address = 0;
	}
	else{
		IfMap_y_Tile_Address = Noy_i*(TIY-NKY+S)-ZERO_PAD;
	}


	// Here, maybe I should check if we have smaller tile because of division reminders
	int x_tile_size = TIX - east_pad * ZERO_PAD - west_pad * ZERO_PAD;

	Input_Loop_Tif: for(int Tif_i=0;Tif_i<TIF;Tif_i++){
#pragma HLS UNROLL factor=1
		IfMap_y_Address = IfMap_y_Tile_Address;
		// Conditional for North Padding
		Input_Loop_North_Pad: for(int i=0; i<ZERO_PAD; i++){
#pragma HLS UNROLL factor=1
			Buf_bank = InBuf_banks_precompute;
			Buf_word = Buf_Wrd_base + InBuf_rows_precompute;
			Input_Loop_Line_1:for(int Tix_i=0;Tix_i<x_tile_size;Tix_i++){
#pragma HLS PIPELINE
				if(north_pad==0){
					InBuf[Buf_bank][Buf_word+row[Tix_i]][col[Tix_i]] = IfMap[Tif_i][IfMap_x_Address+Tix_i][IfMap_y_Address+i];
				}
				else{
					InBuf[Buf_bank][Buf_word+row[Tix_i]][col[Tix_i]] = 0;
				}
			}
			InBuf_write_indexing(&InBuf_rows_count, &InBuf_rows_precompute,
					&InBuf_banks_precompute);
		}
		if(north_pad==0){
			IfMap_y_Address = IfMap_y_Tile_Address + ZERO_PAD;
		}
		// ********************************
		Input_Loop_Tiy_Main: for(int Tiy_i=0;Tiy_i<TIY-2*ZERO_PAD;Tiy_i++){
#pragma HLS UNROLL factor=1
			Buf_bank = InBuf_banks_precompute;
			Buf_word = Buf_Wrd_base + InBuf_rows_precompute;
			Input_Loop_Line_2:for(int Tix_i=0;Tix_i<x_tile_size;Tix_i++){
#pragma HLS PIPELINE
				InBuf[Buf_bank][Buf_word+row[Tix_i]][col[Tix_i]] = IfMap[Tif_i][IfMap_x_Address+Tix_i][IfMap_y_Address];
			}
			//
			IfMap_y_Address++;
			InBuf_write_indexing(&InBuf_rows_count, &InBuf_rows_precompute,
					&InBuf_banks_precompute);
		}
		// Conditional for South Padding
		Input_Loop_South_Pad: for(int i=0; i<ZERO_PAD; i++){
#pragma HLS UNROLL factor=1
//			if(south_pad==0){
//				//Buf_bank = InBuf_banks[i];
//				Buf_bank = InBuf_banks_precompute;
//				Buf_word = Buf_Wrd_base + InBuf_rows_precompute;
//				Input_Loop_Line_3:for(int Tix_i=0;Tix_i<x_tile_size;Tix_i++){
//#pragma HLS PIPELINE
//					InBuf[Buf_bank][Buf_word+row[Tix_i]][col[Tix_i]] = IfMap[Tif_i][IfMap_x_Address+Tix_i][IfMap_y_Address];
//				}
//				IfMap_y_Address++;
//			}
//			else{
//				Buf_bank = InBuf_banks_precompute;
//				Buf_word = Buf_Wrd_base + InBuf_rows_precompute;
//				IF2BUF_label123b:for(int Tix_i=0;Tix_i<x_tile_size;Tix_i++){
//#pragma HLS PIPELINE
//					InBuf[Buf_bank][Buf_word+row[Tix_i]][col[Tix_i]] = 0;
//				}
//			}
			Buf_bank = InBuf_banks_precompute;
			Buf_word = Buf_Wrd_base + InBuf_rows_precompute;
			Input_Loop_Line_3:for(int Tix_i=0;Tix_i<x_tile_size;Tix_i++){
#pragma HLS PIPELINE
				if(south_pad==0){
					InBuf[Buf_bank][Buf_word+row[Tix_i]][col[Tix_i]] = IfMap[Tif_i][IfMap_x_Address+Tix_i][IfMap_y_Address+i];
				}
				else{
					InBuf[Buf_bank][Buf_word+row[Tix_i]][col[Tix_i]] = 0;
				}
			}
			InBuf_write_indexing(&InBuf_rows_count, &InBuf_rows_precompute,
					&InBuf_banks_precompute);
		}
		// ********************************
		Buf_Wrd_base += ROWS_1MAP*WRD_1ROW; // After finishing writing the tile of 1 map to InBuf you add (ROWS_1MAP*WRD_1ROW)
		InBuf_rows_precompute = 0;
		InBuf_rows_count = 0;
		InBuf_banks_precompute = 0;
	}
	// Precomputations
	if(Noy_i==(my_ceil(NOY,TOY)-1)){
		Noy_i = 0;
	}
	else{
		Noy_i++;
	}
}

// Alternative to WT2BUF()
// For Now I use old implementation
void load_WtMap(wt_data_t WtMap[NIF][NOF][NKX][NKY],
	wt_data_t WtBuf[WRD_WTBUF][POF]){
	// Precomputations with inner counter of Nof

	// if(Noy_step == 0 || outer_loop_Nof){
	// 	// Read
	// }

	static int Nof_i = 0;
	//static int Noy_i = 0;
	int WtMap_Of_Address_Base = Nof_i*TOF;
	int Buf_bank=0, Wrd_base=0;

	WT2BUF_label2:for(int Tof_i=0;Tof_i<TOF;Tof_i++){
		WT2BUF_label3:for(int Nif_i=0;Nif_i<NIF;Nif_i++){
			for(int Nky_i=0;Nky_i<NKY;Nky_i++){
				WT2BUF_label4:for(int Nkx_i=0;Nkx_i<NKX;Nkx_i++){
#pragma HLS PIPELINE
					WtBuf[(Wrd_base+Nif_i)*NKX*NKY + Nkx_i + Nky_i*NKX][Buf_bank] = WtMap[Nif_i][WtMap_Of_Address_Base+Tof_i][Nkx_i][Nky_i];
				}
			}
		}
		WtBuf_write_indexing(&Buf_bank, &Wrd_base);
	}

//	if(Noy_i==(my_ceil(NOY,TOY)-1)){
//		if(Nof_i==(my_ceil(NOF,TOF)-1)){
//			Nof_i = 0;
//		}
//		else{
//			Nof_i++;
//		}
//		Noy_i = 0;
//	}
//	else{
//		Noy_i++;
//	}

	if(Nof_i==(my_ceil(NOF,TOF)-1)){
		Nof_i = 0;
	}
	else{
		Nof_i++;
	}
}

// Alternative to BUF_Calc()
void compute_Maps(px_data_t InBuf[POY][WRD_INBUF][POX], wt_data_t WtBuf[WRD_WTBUF][POF],
	px_data_t OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX]){
	Loop_Tof_Toy_Tox_step: for(int step=0;step<my_ceil(TOF,POF)*my_ceil(TOY,POY)*my_ceil(TOX,POX);step++){
// Maybe make buffers stable arrays
#pragma HLS DATAFLOW
		int InBuf_Address_Window, WtBuf_Address_Window;
		data_bool west_pad, east_pad;
		int crnt_wrd, crnt_bank;
		px_data_t rslt_stream[POF][POY][POX];
		Control_Logic(&InBuf_Address_Window, &WtBuf_Address_Window, &west_pad, &east_pad);
		Window_Calculation(InBuf, WtBuf, InBuf_Address_Window, WtBuf_Address_Window, west_pad, east_pad, rslt_stream);
		Control_Logic_2(&crnt_wrd, &crnt_bank);
		PE2BUF(rslt_stream, crnt_wrd, crnt_bank, OutBuf);
	}
}

// Alternative to BUF2OF()
// For Now I use old implementation
void store_Maps(px_data_t OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX],
	px_data_t OfMap[NOF][NOX][NOY]){
	// Precomputations with inner counter of Noy, Nof
	static int Nof_i = 0; static int Noy_i = 0;

	int Buf_bank=0, Buf_word=0;
	int OfMap_x_Address_Base = 0;
	int OfMap_y_Address_Base = Noy_i*TOY;
	int OfMap_Of_Address_Base = Nof_i*TOF;
	// Here, I should check if Tox, Toy, Tof have reminders
	for(int Tof_i=0;Tof_i<TOF;Tof_i++){
		for(int Toy_i=0;Toy_i<TOY;Toy_i++){
			OF2BUF_line_load:for(int Tox_i=0;Tox_i<TOX;Tox_i++){
#pragma HLS PIPELINE
				OfMap[OfMap_Of_Address_Base+Tof_i][OfMap_x_Address_Base+Tox_i][OfMap_y_Address_Base+Toy_i] =
						OutBuf[Buf_bank][(Buf_word+Toy_i)*WRD_1ROW_O+row[Tox_i]][col[Tox_i]];
			}
		}
		OutBuf_read_indexing(&Buf_word, &Buf_bank);
	}

//	if(Noy_i==(my_ceil(NOY,TOY)-1)){
//		if(Nof_i==(my_ceil(NOF,TOF)-1)){
//			Nof_i = 0;
//		}
//		else{
//			Nof_i++;
//		}
//		Noy_i = 0;
//	}
//	else{
//		Noy_i++;
//	}

	if(Nof_i==(my_ceil(NOF,TOF)-1)){
		if(Noy_i==(my_ceil(NOY,TOY)-1)){
			Noy_i = 0;
		}
		else{
			Noy_i++;
		}
		Nof_i = 0;
	}
	else{
		Nof_i++;
	}
}

// 2nd Level of Modules

void Control_Logic(int *InBuf_Address_Window, int *WtBuf_Address_Window,
		data_bool *west_pad, data_bool *east_pad){
	// count Tof, Toy, Tox steps
	// west_pad = (Tox_step==0);
	// east_pad = (Tox_step==my_ceil(TOX,POX)-1);
	// InBuf_Address_Base = Toy_step*WRD_1ROW*S+Tox_step*S;
	// WtBuf_Address_Base = Tof_step*NKX*NKY*TIF;
	static int Tof_step = 0; static int Toy_step = 0; static int Tox_step = 0;

	(*west_pad) = (Tox_step==0);
	(*east_pad) = (Tox_step==my_ceil(TOX,POX)-1);
	(*InBuf_Address_Window) = Toy_step*WRD_1ROW*S+Tox_step*S;
	(*WtBuf_Address_Window) = Tof_step*NKX*NKY*TIF;

	if(Tox_step == my_ceil(TOX,POX)-1){
		Tox_step = 0;
		if(Toy_step == my_ceil(TOY,POY)-1){
			Toy_step = 0;
			if(Tof_step == my_ceil(TOF,POF)-1){
				Tof_step = 0;
			}
			else{
				Tof_step++;
			}
		}
		else{
			Toy_step++;
		}
	}
	else{
		Tox_step++;
	}
}

void Control_Logic_2(int *crnt_wrd, int *crnt_bank){
	static int Tof_step = 0; static int Toy_step = 0; static int Tox_step = 0;
	(*crnt_wrd) = Tox_step + Toy_step*POY*my_ceil(TOX,POX)
                + ((Tof_step*POF)/OUTBUF_NUM)*my_ceil(TOX,POX)*TOY;
    (*crnt_bank) = (Tof_step*POF)%OUTBUF_NUM;
	if(Tox_step == my_ceil(TOX,POX)-1){
		Tox_step = 0;
		if(Toy_step == my_ceil(TOY,POY)-1){
			Toy_step = 0;
			if(Tof_step == my_ceil(TOF,POF)-1){
				Tof_step = 0;
			}
			else{
				Tof_step++;
			}
		}
		else{
			Toy_step++;
		}
	}
	else{
		Tox_step++;
	}
}

void Window_Calculation(px_data_t InBuf[POY][WRD_INBUF][POX], wt_data_t WtBuf[WRD_WTBUF][POF],
		int InBuf_Address_Window, int WtBuf_Address_Window, data_bool west_pad, data_bool east_pad,
		px_data_t rslt_stream[POF][POY][POX]){
	Nif_Nky_Nkx_Loop: for(int i=0;i<NIF*NKY*NKX;i++){
//#pragma HLS STABLE variable=InBuf_Address_Window
//#pragma HLS STABLE variable=east_pad
//#pragma HLS STABLE variable=InBuf
//#pragma HLS STABLE variable=WtBuf
#pragma HLS DATAFLOW
//#pragma HLS STABLE variable=InBuf
		// Variable Declaration
		px_data_t px_stream[POY][POX];
		wt_data_t wt_stream[POF];
		INBUF2PE_shell(InBuf, InBuf_Address_Window, west_pad, east_pad, px_stream);
		WTBUF2PE(WtBuf, WtBuf_Address_Window, wt_stream);
		PE(px_stream, wt_stream, rslt_stream);
	}
}

// For now, I just replace old implementation with very few adjustments.
void PE2BUF(px_data_t rslt_stream[POF][POY][POX],
			int crnt_wrd, int crnt_bank,
			px_data_t OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX]){
#pragma HLS PIPELINE II=3
	// Declarations
	static int layer_no = 0; // To be implemented later
	// Will create a Tile_steps array that has the appropriate "my_ceil(T*,P*)" values
	static int Tof_step = 0; static int Toy_step = 0; static int Tox_step = 0;
	px_data_t PE_results_copy[POF][POY][POX];
	int Pof_last = (TOF%POF!=0)&&(Tof_step==TOF/POF);
	int Poy_last = (TOY%POY!=0)&&(Toy_step==TOY/POY);
	int Pox_last = (TOX%POX!=0)&&(Tox_step==TOX/POX);
	int POF_count = (Pof_last)? TOF%POF : POF;
	int POY_count = (Poy_last)? TOY%POY : POY;
	int POX_count = (Pox_last)? TOX%POX : POX;
//	int POF_count = POF;
//	int POY_count = POY;
//    int POX_count = POX;
	for(int Pof_i=0;Pof_i<POF;Pof_i++){
#pragma HLS UNROLL
		for(int Poy_i=0;Poy_i<POY;Poy_i++){
#pragma HLS UNROLL
			for(int Pox_i=0;Pox_i<POX;Pox_i++){
#pragma HLS UNROLL
				PE_results_copy[Pof_i][Poy_i][Pox_i] = rslt_stream[Pof_i][Poy_i][Pox_i];
			}
		}
	}

    Loop_POY_count: for(int Poy_i=0;Poy_i<POY;Poy_i++){
        Loop_my_ceil: for(int index2=0;index2<my_ceil(POF_count,OUTBUF_NUM);index2++){
        	for(int OutBuf_Num_i=0;OutBuf_Num_i<OUTBUF_NUM;OutBuf_Num_i++){
#pragma HLS UNROLL
				for(int Pox_i=0;Pox_i<POX;Pox_i++){
#pragma HLS UNROLL
					if(Poy_i<POY_count && index2*OUTBUF_NUM+OutBuf_Num_i<POF_count
								&& Pox_i<POX_count){
							OutBuf[(crnt_bank+OutBuf_Num_i)%OUTBUF_NUM]
								   [Poy_i*my_ceil(TOX,POX) + crnt_wrd
									+index2*my_ceil(TOX,POX)*TOY
									+((crnt_bank+OutBuf_Num_i)/OUTBUF_NUM)*my_ceil(TOX,POX)*TOY]
										   [Pox_i] =
												   PE_results_copy[(OutBuf_Num_i+index2*OUTBUF_NUM)][Poy_i][Pox_i];

//						else{
//							OutBuf[(crnt_bank+OutBuf_Num_i)%OUTBUF_NUM]
//								[index2*my_ceil(TOX,POX)*TOY + Poy_i*my_ceil(TOX,POX) + crnt_wrd
//								+((crnt_bank+OutBuf_Num_i)/OUTBUF_NUM)*my_ceil(TOX,POX)*TOY]
//								[Pox_i] = 0;
//						}
					}
				}
			}
        }
    }
    if(Tox_step == my_ceil(TOX,POX)-1){
        if(Toy_step == my_ceil(TOY,POY)-1){
            if(Tof_step == my_ceil(TOF,POF)-1){
            	Tof_step = 0;
            }
            else{
            	Tof_step++;
            }
        	Toy_step = 0;
        }
        else{
        	Toy_step++;
        }
    	Tox_step = 0;
    }
    else{
    	Tox_step++;
    }
}

// 3rd Level of Modules

void INBUF2PE_shell(px_data_t InBuf[POY][WRD_INBUF][POX],
		int InBuf_Address_Window, data_bool west_pad, data_bool east_pad,
		px_data_t px_stream[POY][POX]){
#pragma HLS PIPELINE II=1
    static hls::stream<px_data_t> fifo_arr[POY-1][POX]; // FIFOs of BUF2PE data bus
#pragma HLS STREAM variable=fifo_arr depth=8
#pragma HLS ARRAY_PARTITION variable=fifo_arr complete dim=1
#pragma HLS ARRAY_PARTITION variable=fifo_arr complete dim=2
    INBUF2PE(InBuf, fifo_arr, InBuf_Address_Window, west_pad, east_pad, px_stream);
}

// For now, I just replace old implementation with very few adjustments.
void INBUF2PE(px_data_t InBuf[POY][WRD_INBUF][POX], hls::stream<px_data_t> fifo_arr[POY-1][POX],
	int InBuf_Address_Window, data_bool west_pad, data_bool east_pad,
	px_data_t px_stream[POY][POX]){
#pragma HLS INLINE off
	px_data_t InternalReg[POY][POX]; // Internal Registers of BUF2PE data bus
//#pragma HLS STREAM variable=InternalReg
#pragma HLS ARRAY_PARTITION variable=InternalReg complete dim=1
#pragma HLS ARRAY_PARTITION variable=InternalReg complete dim=2
	BUF2InternalReg(InBuf, InBuf_Address_Window, west_pad, east_pad,
		fifo_arr, InternalReg);
	Reg2FIFO_PE(InternalReg, fifo_arr, px_stream);
}

void WTBUF2PE(wt_data_t WtBuf[WRD_WTBUF][POF],
	int WtBuf_Address_Window,
	wt_data_t wt_stream[POF]){
#pragma HLS PIPELINE II=1
	static int Nif_i = 0; static int Nky_i = 0; static int Nkx_i = 0;
	for(int count=0;count<POF;count++){
#pragma HLS unroll
		wt_stream[count] = WtBuf[WtBuf_Address_Window + Nif_i*NKX*NKY + Nky_i*NKX + Nkx_i][count];
	}

	// Counter Calculations
	if(Nkx_i==NKX-1){
		if(Nky_i==NKY-1){
			if(Nif_i==NIF-1){
				Nif_i = 0;
			}
			else{
				Nif_i++;
			}
			Nky_i = 0;
		}
		else{
			Nky_i++;
		}
		Nkx_i = 0;
	}
	else{
		Nkx_i++;
	}
}

void PE(px_data_t px_stream[POY][POX], wt_data_t wt_stream[POF],
		px_data_t rslt_stream[POF][POY][POX]){
#pragma HLS INLINE off
#pragma HLS PIPELINE II=1
	static px_data_t tmp_rslt[POF][POY][POX]; // Intermediate results
#pragma HLS ARRAY_PARTITION variable=tmp_rslt complete dim=1
#pragma HLS ARRAY_PARTITION variable=tmp_rslt complete dim=2
#pragma HLS ARRAY_PARTITION variable=tmp_rslt complete dim=3

	// "State" variables to know when each pixel calculation starts and ends
	static int state = 0; static data_bool start = 1; static data_bool end = 0;

	for(int Pof_i=0;Pof_i<POF;Pof_i++){
#pragma HLS UNROLL
		for(int Poy_i=0;Poy_i<POY;Poy_i++){
#pragma HLS UNROLL
			for(int Pox_i=0;Pox_i<POX;Pox_i++){
#pragma HLS UNROLL
				if( start ){
					tmp_rslt[Pof_i][Poy_i][Pox_i] = px_data_t(0);//'0000000000000000';
				}
				tmp_rslt[Pof_i][Poy_i][Pox_i] += px_stream[Poy_i][Pox_i] * wt_stream[Pof_i];
				if( end ){
					rslt_stream[Pof_i][Poy_i][Pox_i] = tmp_rslt[Pof_i][Poy_i][Pox_i];
				}
			}
		}
	}
	// State Calculation
	if(state == NIF*NKX*NKY-1){
		start = 1;
		end = 0;
		state = 0;
	}
	else if(state == NIF*NKX*NKY-2){
		start = 0;
		end = 1;
		state++;
	}
	else{
		start = 0;
		end = 0;
		state++;
	}
}

// 4th Level of Modules

void BUF2InternalReg(px_data_t InBuf[POY][WRD_INBUF][POX], 
		int InBuf_Address_Window, data_bool west_pad, data_bool east_pad,
		hls::stream<px_data_t> fifo_arr[POY-1][POX],
		px_data_t InternalReg[POY][POX]){
#pragma HLS INLINE
//#pragma HLS pipeline II=1 rewind style=stp
//#pragma HLS pipeline II=1 rewind
	// For Poy=POY-1 we always write from InBuf, but different indexing at Nky>=S.
	// For Poy<POY-1 we read from InBuf for Nky<S and from FIFOs for Nky>=S.
	px_data_t tmpInBuf0[POY];
#pragma HLS ARRAY_PARTITION variable=tmpInBuf0 type=complete
	px_data_t tmpInBuf1[POY];
#pragma HLS ARRAY_PARTITION variable=tmpInBuf1 type=complete
	px_data_t tmpInBuf2[POY];
#pragma HLS ARRAY_PARTITION variable=tmpInBuf2 type=complete
	px_data_t tmpFIFO[POY-1][POX];
#pragma HLS ARRAY_PARTITION variable=tmpFIFO dim=1 type=complete
#pragma HLS ARRAY_PARTITION variable=tmpFIFO dim=2 type=complete
	static int InBuf_Address_offset[POY] = { 1, 0, -1};
#pragma HLS ARRAY_PARTITION variable=InBuf_Address_offset type=complete
	static data_bool state_Buf2InternalReg = 1;
	static int mux_state = 0;
	static data_bool condition1a = 0; static data_bool condition1b = 0; static data_bool condition1c = 0;
	static data_bool condition2 = 1;
	static int bank = 0;

	for(int Poy_i=0;Poy_i<POY;Poy_i++){
#pragma HLS UNROLL
		// Ad Hock solution for Padding
		// For West Pad: Nkx_i=0 and Pox_i=0 and collumn_2=-1
		// For East Pad: Nkx_i = NKX-1 and Pox_i=POX and collumn_0=2
//		if( east_pad && condition1 ){ //Pox=0
//			tmpInBuf0[Poy_i] = 0;
//		}
//		else{
//			//tmpInBuf0[Poy_i] = InBuf[Poy_i][InBuf_Address + collumn0[Nkx_i]][0];
//			tmpInBuf0[Poy_i] = InBuf[Poy_i][InBuf_Address_offset[0] + InBuf_Address_Window][0];
//		}
		tmpInBuf0[Poy_i] = InBuf[Poy_i][InBuf_Address_offset[0] + InBuf_Address_Window][0];
		tmpInBuf1[Poy_i] = InBuf[Poy_i][InBuf_Address_offset[1] + InBuf_Address_Window][1];
		//tmpInBuf1[Poy_i] = InBuf[Poy_i][InBuf_Address + collumn1[Nkx_i]][1];
//		if( east_pad && condition1 ){
//			tmpInBuf1[Poy_i] = 0;
//		}
//		else{
//			tmpInBuf1[Poy_i] = InBuf[Poy_i][InBuf_Address_offset[1] + InBuf_Address_Window][1];
//		}
		//if( (west_pad && condition2) || (east_pad && condition1) ){
		if( west_pad && condition2 ){
			tmpInBuf2[Poy_i] = 0;
		}
		else{
			//tmpInBuf2[Poy_i] = InBuf[Poy_i][InBuf_Address + collumn2[Nkx_i]][2];
			tmpInBuf2[Poy_i] = InBuf[Poy_i][InBuf_Address_offset[2] + InBuf_Address_Window][2];
		}
	}
	if(state_Buf2InternalReg == 0){
		for(int Poy_i=0;Poy_i<POY-1;Poy_i++){
#pragma HLS UNROLL
			for(int Pox_i=0;Pox_i<POX;Pox_i++){
#pragma HLS UNROLL
				fifo_arr[Poy_i][Pox_i].read_nb(tmpFIFO[Poy_i][Pox_i]);
			}
		}
	}
	else{
		for(int Poy_i=0;Poy_i<POY-1;Poy_i++){
#pragma HLS UNROLL
			for(int Pox_i=0;Pox_i<POX;Pox_i++){
#pragma HLS UNROLL
				tmpFIFO[Poy_i][Pox_i] = 0;
			}
		}
	}

	if(state_Buf2InternalReg == 1){
		for(int Poy_i=0;Poy_i<POY;Poy_i++){
#pragma HLS UNROLL
			if(mux_state == 0){
				InternalReg[Poy_i][2] = tmpInBuf0[Poy_i];
				InternalReg[Poy_i][1] = tmpInBuf1[Poy_i];
				InternalReg[Poy_i][0] = tmpInBuf2[Poy_i];
			}
			else if(mux_state == 1){
				InternalReg[Poy_i][0] = tmpInBuf0[Poy_i];
				InternalReg[Poy_i][2] = tmpInBuf1[Poy_i];
				InternalReg[Poy_i][1] = tmpInBuf2[Poy_i];
			}
			else{
				InternalReg[Poy_i][1] = tmpInBuf0[Poy_i];
				InternalReg[Poy_i][0] = tmpInBuf1[Poy_i];
				InternalReg[Poy_i][2] = tmpInBuf2[Poy_i];
			}
		}
	}
	else{
		for(int Poy_i=0;Poy_i<POY-1;Poy_i++){
#pragma HLS UNROLL
			for(int Pox_i=0;Pox_i<POX;Pox_i++){
#pragma HLS UNROLL
			InternalReg[Poy_i][Pox_i] = tmpFIFO[Poy_i][Pox_i];
			}
		}
		if(mux_state == 0){
			InternalReg[POY-1][2] = tmpInBuf0[bank];
			InternalReg[POY-1][1] = tmpInBuf1[bank];
			InternalReg[POY-1][0] = tmpInBuf2[bank];
		}
		else if(mux_state == 1){
			InternalReg[POY-1][0] = tmpInBuf0[bank];
			InternalReg[POY-1][2] = tmpInBuf1[bank];
			InternalReg[POY-1][1] = tmpInBuf2[bank];
		}
		else{
			InternalReg[POY-1][1] = tmpInBuf0[bank];
			InternalReg[POY-1][0] = tmpInBuf1[bank];
			InternalReg[POY-1][2] = tmpInBuf2[bank];
		}
	}
	for(int Poy_i=0;Poy_i<POY;Poy_i++){
#pragma HLS UNROLL
		if(east_pad && condition1a){
			InternalReg[Poy_i][0] = 0;
		}
//		else{
//			InternalReg[Poy_i][0] = 0;
//		}
		if(east_pad && condition1b){
			InternalReg[Poy_i][1] = 0;
		}
//		else{
//
//		}
		if(east_pad && condition1c){
			InternalReg[Poy_i][2] = 0;
		}
//		else{
//
//		}
	}

	//****************************************************************************************************
//     // Both cases for Poy<POY-1
// 	for(int Poy_i=0;Poy_i<POY-1;Poy_i++){
// #pragma HLS UNROLL
// 		// Ad Hock solution for Padding
// 		// For West Pad: Nkx_i=0 and Pox_i=0 and collumn_2=-1
// 		// For East Pad: Nkx_i = NKX-1 and Pox_i=POX and collumn_0=2
// 		if(state_Buf2InternalReg){
// 			if( condition1 ){ //Pox=0
// 				tmpInBuf0[Poy_i] = 0;
// 			}
// 			else{
// 				//tmpInBuf0[Poy_i] = InBuf[Poy_i][InBuf_Address + collumn0[Nkx_i]][0];
// 				tmpInBuf0[Poy_i] = InBuf[Poy_i][InBuf_Address[0]][0];
// 			}
// 			//tmpInBuf1[Poy_i] = InBuf[Poy_i][InBuf_Address + collumn1[Nkx_i]][1];
// 			tmpInBuf1[Poy_i] = InBuf[Poy_i][InBuf_Address[1]][1];
// 			if( condition2 ){
// 				tmpInBuf2[Poy_i] = 0;
// 			}
// 			else{
// 				//tmpInBuf2[Poy_i] = InBuf[Poy_i][InBuf_Address + collumn2[Nkx_i]][2];
// 				tmpInBuf2[Poy_i] = InBuf[Poy_i][InBuf_Address[2]][2];
// 			}

// 			if(mux_state == 0){
// 				InternalReg[Poy_i][2] = tmpInBuf0[Poy_i];
// 				InternalReg[Poy_i][1] = tmpInBuf1[Poy_i];
// 				InternalReg[Poy_i][0] = tmpInBuf2[Poy_i];
// 			}
// 			else if(mux_state == 1){
// 				InternalReg[Poy_i][0] = tmpInBuf0[Poy_i];
// 				InternalReg[Poy_i][2] = tmpInBuf1[Poy_i];
// 				InternalReg[Poy_i][1] = tmpInBuf2[Poy_i];
// 			}
// 			else{
// 				InternalReg[Poy_i][1] = tmpInBuf0[Poy_i];
// 				InternalReg[Poy_i][0] = tmpInBuf1[Poy_i];
// 				InternalReg[Poy_i][2] = tmpInBuf2[Poy_i];
// 			}
// 		}
// 		else{
// 			for(int Pox_i=0;Pox_i<POX;Pox_i++){
// #pragma HLS UNROLL
// 				fifo_arr[Poy_i][Pox_i].read(InternalReg[Poy_i][Pox_i]);
// 			}
// 		}
// 	}
// 	// Both cases for Poy=POY-1
	
// 	if(state_Buf2InternalReg){
// 		bank_index = POY-1;
// 	}
// 	else{
// 		bank_index = bank;
// 	}
// 	if( condition1 ){ //Pox=0
// 		tmpInBuf0[POY-1] = 0;
// 	}
// 	else{
// 		//tmpInBuf0[POY-1] = InBuf[bank_index][InBuf_Address + collumn0[Nkx_i]][0];
// 		tmpInBuf0[POY-1] = InBuf[bank_index][InBuf_Address[0]][0];
// 	}
// 	//tmpInBuf1[POY-1] = InBuf[bank_index][InBuf_Address + collumn1[Nkx_i]][1];
// 	tmpInBuf1[POY-1] = InBuf[bank_index][InBuf_Address[1]][1];
// 	if( condition2 ){
// 		tmpInBuf2[POY-1] = 0;
// 	}
// 	else{
// 		//tmpInBuf2[POY-1] = InBuf[bank_index][InBuf_Address + collumn2[Nkx_i]][2];
// 		tmpInBuf2[POY-1] = InBuf[bank_index][InBuf_Address[2]][2];
// 	}

// 	if(mux_state == 0){
// 		InternalReg[POY-1][2] = tmpInBuf0[POY-1];
// 		InternalReg[POY-1][1] = tmpInBuf1[POY-1];
// 		InternalReg[POY-1][0] = tmpInBuf2[POY-1];
// 	}
// 	else if(mux_state == 1){
// 		InternalReg[POY-1][0] = tmpInBuf0[POY-1];
// 		InternalReg[POY-1][2] = tmpInBuf1[POY-1];
// 		InternalReg[POY-1][1] = tmpInBuf2[POY-1];
// 	}
// 	else{
// 		InternalReg[POY-1][1] = tmpInBuf0[POY-1];
// 		InternalReg[POY-1][0] = tmpInBuf1[POY-1];
// 		InternalReg[POY-1][2] = tmpInBuf2[POY-1];
// 	}
	BUF2InternalReg_Controller(InBuf_Address_offset,
		&state_Buf2InternalReg, &mux_state, &condition1a, &condition1b, &condition1c, &condition2, &bank);
}

void Reg2FIFO_PE(px_data_t InternalReg[POY][POX], hls::stream<px_data_t> fifo_arr[POY-1][POX],
		px_data_t px_stream[POY][POX]){
#pragma HLS INLINE
//#pragma HLS pipeline II=1 rewind style=stp
//#pragma HLS pipeline II=1 rewind
//#pragma HLS loop_merge
	static int Nif_i = 0; 	static int Nky_i = 0; 	static int Nkx_i = 0;
	static int state_Reg2FIFO_PE = 1;
	// Forward to FIFOs and PEs
    for(int Pox_i=0;Pox_i<POX;Pox_i++){
#pragma HLS UNROLL
    	for(int Poy_i=0;Poy_i<POY;Poy_i++){
#pragma HLS UNROLL
    		px_stream[Poy_i][Pox_i] = InternalReg[Poy_i][Pox_i];
			if( (Poy_i!=0) && state_Reg2FIFO_PE ){
    			fifo_arr[Poy_i-1][Pox_i].write_nb(InternalReg[Poy_i][Pox_i]);
			}
    	}
    }

//    for(int Pox_i=0;Pox_i<POX;Pox_i++){
//#pragma HLS UNROLL
//    	for(int Poy_i=0;Poy_i<POY;Poy_i++){
//#pragma HLS UNROLL
////			if( (Poy_i!=0) && state_Reg2FIFO_PE ){
////    			fifo_arr[Poy_i-1][Pox_i].write_nb(InternalReg[Poy_i][Pox_i]);
////			}
//			px_stream[Poy_i][Pox_i] = InternalReg[Poy_i][Pox_i];
//    	}
//    }
//
//	for(int Pox_i=0;Pox_i<POX;Pox_i++){
//#pragma HLS UNROLL
//		for(int Poy_i=1;Poy_i<POY;Poy_i++){
//#pragma HLS UNROLL
//        	if(state_Reg2FIFO_PE){
//        		fifo_arr[Poy_i-1][Pox_i].write_nb(InternalReg[Poy_i][Pox_i]);
//        	}
//        }
//    }


	if(Nkx_i == (NKX-1) ){
		Nkx_i = 0;
		if(Nky_i == (NKY-1) ){
			Nky_i = 0;
			if(Nif_i == (NIF-1) ){
				Nif_i = 0;
			}
			else{
				Nif_i++;
			}
		}
		else{
			Nky_i++;
		}
	}
	else{
		Nkx_i++;
	}
	if( Nky_i==(NKY-S) ){
		state_Reg2FIFO_PE = 0;
	}
	else if( Nky_i==0 ){
		state_Reg2FIFO_PE = 1;
	}
	else{
		state_Reg2FIFO_PE = state_Reg2FIFO_PE;
	}
}

// Inlined Modules

void InBuf_read_Controller(int InBuf_Address_Window, data_bool west_pad, data_bool east_pad, int InBuf_Address[POY], data_bool *state_Buf2InternalReg, int *mux_state,
	data_bool *condition1, data_bool *condition2, int *bank,
	data_bool *state_Reg2FIFO_PE){
	#pragma HLS PIPELINE
	static int Nif_i = 0; 	static int Nky_i = 0; 	static int Nkx_i = 0;
	static int mux_state_reg = 0;
	static int rows_reg = 0; static int bank_reg = 0; static int s_rows_reg = 0;
	int rows_index;
//    const int collumn0[NKX] = {  1,0,1,2 };
//#pragma HLS ARRAY_PARTITION variable=collumn0 type=complete
//    const int collumn1[NKX] = {  0,1,0,1 };
//#pragma HLS ARRAY_PARTITION variable=collumn1 type=complete
//    const int collumn2[NKX] = { -1,0,1,0 };
//#pragma HLS ARRAY_PARTITION variable=collumn2 type=complete
    const int collumn[POY][NKX] = {
    		{  1,0,1,2 },
			{  0,1,0,1 },
			{ -1,0,1,0 }
    };
#pragma HLS ARRAY_PARTITION dim=1 type=complete variable=collumn
#pragma HLS ARRAY_PARTITION dim=2 type=complete variable=collumn
	// BUF2InternalReg
	if(Nky_i<S){
		rows_index = Nky_i;
	}
	else{
		rows_index = (S*(s_rows_reg+1)+rows_reg);
	}
	//(*InBuf_Address) = InBuf_Address_Window + Nif_i*ROWS_1MAP*WRD_1ROW+ Nky_i*WRD_1ROW;
	for(int Poy_i=0;Poy_i<POY;Poy_i++){
#pragma HLS UNROLL
		InBuf_Address[Poy_i] = InBuf_Address_Window + Nif_i*ROWS_1MAP*WRD_1ROW+ rows_index*WRD_1ROW + collumn[Poy_i][Nkx_i];
	}
	//(*InBuf_Address) = InBuf_Address_Window + Nif_i*ROWS_1MAP*WRD_1ROW+ rows_index*WRD_1ROW;
	(*state_Buf2InternalReg) = (Nky_i<S);
	(*mux_state) = mux_state_reg;
	if(Nkx_i==NKX-1){
		mux_state_reg = 0;
	}
	else if(mux_state_reg == POX-1){
		mux_state_reg = 0;
	}
	else{
		mux_state_reg++;
	}
	(*condition1) = east_pad && Nkx_i==(NKX-1);
	(*condition2) = west_pad && Nkx_i==0;
	(*bank) = bank_reg;
	if(Nkx_i==NKX-1 && Nky_i==NKY-1){
		rows_reg = 0;bank_reg = 0;s_rows_reg = 0;
	}
	else if(Nky_i>=S && (Nkx_i==NKX-1)){
		if( rows_reg == S-1){
			rows_reg = 0;
			if(NKY-S>POX){
				if(bank_reg == POX-1){ // Here, considering the specific values of parameters, it may be unnecessary.
								   //Probably the compiler doesn't know it. So maybe specific ifs that can be ignored should be given
					bank_reg = 0; s_rows_reg++;
				}
				else{
					bank_reg++;
				}
			}
			else{
				bank_reg++;
			}
		}
		else{
			rows_reg++;
		}
	}
	// Reg2FIFOPE
	(*state_Reg2FIFO_PE) = ( Nky_i<(NKY-S) );
	
	// Counter Calculations
	if(Nkx_i == (NKX-1) ){
		Nkx_i = 0;
		if(Nky_i == (NKY-1) ){
			Nky_i = 0;
			if(Nif_i == (NIF-1) ){
				Nif_i = 0;
			}
			else{
				Nif_i++;
			}
		}
		else{
			Nky_i++;
		}
	}
	else{
		Nkx_i++;
	}
}

void BUF2InternalReg_Controller(int InBuf_Address_offset[POY],
			data_bool *state_Buf2InternalReg, int *mux_state, 
			data_bool *condition1a, data_bool *condition1b, data_bool *condition1c,
			data_bool *condition2, int *bank){
	static int Nif_i = 0; 	static int Nky_i = 0; 	static int Nkx_i = 1;
	static int rows_reg = 0; static int bank_reg = 0; static int s_rows_reg = 0;
	static int mux_state_reg = 1;
	const int collumn[POY][NKX] = {
    		{  1,0,1,2 },
			{  0,1,0,1 },
			{ -1,0,1,0 }
    };
	int rows_index;





	if(Nky_i<S){
		rows_index = Nky_i;
	}
	else{
		rows_index = (S*(s_rows_reg+1)+rows_reg);
	}
	// Outputs
	for(int Poy_i=0;Poy_i<POY;Poy_i++){
#pragma HLS UNROLL
		InBuf_Address_offset[Poy_i] = Nif_i*ROWS_1MAP*WRD_1ROW+ rows_index*WRD_1ROW + collumn[Poy_i][Nkx_i];
	}
	(*state_Buf2InternalReg) = (Nky_i<S);
	(*mux_state) = mux_state_reg;
	// We could replace modulo operation with constants for each layer
	// Zero pad=1 has been asssumed
	(*condition1a) = ( Nkx_i==(NKX-1) && (TOX%POX == 1) ); // all elements
	(*condition1b) = ( Nkx_i==(NKX-1) && (TOX%POX == 2) ); // last two elements
	(*condition1c) = ( Nkx_i==(NKX-1) && (TOX%POX == 0) ); // only last element
	(*condition2) = ( Nkx_i==0 );
	(*bank) = bank_reg;

	if(Nkx_i==NKX-1){
		mux_state_reg = 0;
	}
	else if(mux_state_reg == POX-1){
		mux_state_reg = 0;
	}
	else{
		mux_state_reg++;
	}
	if(Nkx_i==NKX-1 && Nky_i==NKY-1){
		rows_reg = 0;bank_reg = 0;s_rows_reg = 0;
	}
	else if(Nky_i>=S && (Nkx_i==NKX-1)){
		if( rows_reg == S-1){
			rows_reg = 0;
			if(NKY-S>POX){
				if(bank_reg == POX-1){ // Here, considering the specific values of parameters, it may be unnecessary.
								   //Probably the compiler doesn't know it. So maybe specific ifs that can be ignored should be given
					bank_reg = 0; s_rows_reg++;
				}
				else{
					bank_reg++;
				}
			}
			else{
				bank_reg++;
			}
		}
		else{
			rows_reg++;
		}
	}


	// Counter Calculations
	if(Nkx_i == (NKX-1) ){
		Nkx_i = 0;
		if(Nky_i == (NKY-1) ){
			Nky_i = 0;
			if(Nif_i == (NIF-1) ){
				Nif_i = 0;
			}
			else{
				Nif_i++;
			}
		}
		else{
			Nky_i++;
		}
	}
	else{
		Nkx_i++;
	}
}

void InBuf_read_indexing(int Nky_i, int Nkx_i,
						int *rows, int *bank, int *s_rows){
#pragma HLS INLINE
	if(Nkx_i==NKX-1 && Nky_i==NKY-1){
		(*rows) = 0;(*bank) = 0;(*s_rows) = 0;
	}
	else if(Nky_i>=S && (Nkx_i==NKX-1)){
		if( (*rows) == S-1){
			(*rows) = 0;
			if(NKY-S>POX){
				if((*bank) == POX-1){ // Here, considering the specific values of parameters, it may be unnecessary.
								   //Probably the compiler doesn't know it. So maybe specific ifs that can be ignored should be given
					(*bank) = 0; (*s_rows)++;
				}
				else{
					(*bank)++;
				}
			}
			else{
				(*bank)++;
			}
		}
		else{
			(*rows)++;
		}
	}
}

void InBuf_write_indexing(int *InBuf_rows_count, int *InBuf_rows_precompute,
		int *InBuf_banks_precompute){
	if( (*InBuf_rows_count) == S-1){
		(*InBuf_rows_count) = 0;
		if( (*InBuf_banks_precompute) == POY-1){
			(*InBuf_banks_precompute) = 0;
			(*InBuf_rows_precompute) += WRD_1ROW;
		}
		else{
			(*InBuf_banks_precompute)++;
			(*InBuf_rows_precompute) -= (S-1)*WRD_1ROW;
		}
	}
	else{
		(*InBuf_rows_count)++;
		(*InBuf_rows_precompute) += WRD_1ROW;
	}
}

void WtBuf_write_indexing(int *Buf_bank, int *Wrd_base){
	if( (*Buf_bank) == POF-1){
		(*Buf_bank) = 0;
		(*Wrd_base) += NIF;
	}
	else{
		(*Buf_bank)++;
	}
}

void OutBuf_read_indexing(int *OutBuf_rows_precompute,
		int *OutBuf_banks_precompute){
	if( (*OutBuf_banks_precompute) == OUTBUF_NUM-1){
		(*OutBuf_banks_precompute) = 0;
		(*OutBuf_rows_precompute) += (TOY);
	}
	else{
		(*OutBuf_banks_precompute)++;
	}
}

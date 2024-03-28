#include "header.h"
#include <iostream>
#include <iomanip>

const int Nif = NIF;
const int Nix = NIX-2*ZERO_PAD;// // Always
// const int Niy = TIY-ZERO_PAD; // If first or last vertically
// const int Niy = TIY; // If in middle
const int Niy = NIY-2*ZERO_PAD; // If Niy=Tiy
const int Nof = NOF;
const int Nkx = NKX;
const int Nky = NKY;
const int Nox = NOX;
const int Noy = NOY;

void IF2BUF_golden(px_data_t IfMap[NIF][NIX-2*ZERO_PAD][NIY-2*ZERO_PAD],
		px_data_t InBuf[POY][WRD_INBUF][POX],
		int IfMap_y_Address, data_bool north_pad, data_bool south_pad){
// Newest version with full loops
	Init_Loop_1: for(int index=0; index<WRD_INBUF;index++){
		Init_Loop_2: for(int Poy_i=0; Poy_i<POY;Poy_i++){
			Init_Loop_3: for(int Pox_i=0;Pox_i<POX;Pox_i++){
				InBuf[Poy_i][index][Pox_i] = 0;
			}
		}
	}
	IF2BUF_Tif:for(int Tif_i=0;Tif_i<TIF;Tif_i++){
		for(int index=0;index<my_ceil(my_ceil(TIY,S),POY);index++){
			for(int S_i=0;S_i<S;S_i++){
				for(int Word1_i=0;Word1_i<WRD_1ROW;Word1_i++){
					Poy_Loop_to_Buf: for(int Poy_i=0;Poy_i<POY;Poy_i++){
						if(index!=0 || Poy_i*S+S_i>=north_pad*ZERO_PAD){
							Pox_Loop_to_Buf: for(int Pox_i=0;Pox_i<POX;Pox_i++){
								if( //Word1_i*POX+Pox_i<TIX-2*ZERO_PAD &&
								Poy_i*S+S_i+index*POY*S<TIY-south_pad*ZERO_PAD &&
										Poy_i*S+S_i+index*POY*S-north_pad*ZERO_PAD>=0){
									InBuf[Poy_i][Word1_i + S_i*WRD_1ROW + index*S*WRD_1ROW
										+Tif_i*ROWS_1MAP*WRD_1ROW][Pox_i]
										= IfMap[Tif_i]
										[Pox_i+Word1_i*POX]
										[IfMap_y_Address+Poy_i*S+S_i+index*POY*S-ZERO_PAD];
								}
							}
						}
					}
				}
			}
		}
	}
}


void print_ComparedMap(float Compared[NOF][NOX][NOY]){
	std::cout << "***  Printing Boolean Comparison of Output Feature Map  ***" << std::endl;
	std::cout << std::setprecision(5) << std::fixed;
	for(int Nof_i=0;Nof_i<Nof;Nof_i++){
		std::cout << Nof_i+1 << " Map" << std::endl;
		for(int Noy_i=0;Noy_i<Noy;Noy_i++){
			for(int Nox_i=0;Nox_i<Nox;Nox_i++){
				std::cout << std::setw(20) << Compared[Nof_i][Nox_i][Noy_i];
			}
		std::cout << " " << std::endl;
		}
	}
}
void print_IfMap(px_data_t IfMap[Nif][Nix][Niy]){
	std::cout << "***  Printing Input Feature Map  ***" << std::endl;
	for(int Nif_i=0;Nif_i<Nif;Nif_i++){
		std::cout << Nif_i+1 << " Map" << std::endl;
		for(int Niy_i=0;Niy_i<Niy;Niy_i++){
			for(int Nix_i=0;Nix_i<Nix;Nix_i++){
				std::cout << IfMap[Nif_i][Nix_i][Niy_i] << "     " ;
			}
		std::cout << " " << std::endl;
		}
	}
}

void print_InBuf(px_data_t InBuf[POY][WRD_INBUF][POX]){
	std::cout << "***  Printing Input Buffers  ***" << std::endl;
	for(int Wrd_InBuf_i=0;Wrd_InBuf_i<WRD_INBUF;Wrd_InBuf_i++){
		for(int Poy_i=0;Poy_i<POY;Poy_i++){
			for(int Pox_i=0;Pox_i<POX;Pox_i++){
				std::cout << std::setw(6) << InBuf[Poy_i][Wrd_InBuf_i][Pox_i] << "     ";
			}
		std::cout << "|      ";
		}
	std::cout << " " << std::endl;
	}
}

void IF2BUF_software(px_data_t IfMap[NIF][NIX-2*ZERO_PAD][NIY-2*ZERO_PAD],
		px_data_t InBuf[POY][WRD_INBUF][POX],
		int IfMap_y_Address, data_bool north_pad, data_bool south_pad){
// Newest version with full loops
	Init_Loop_1: for(int index=0; index<WRD_INBUF;index++){
		Init_Loop_2: for(int Poy_i=0; Poy_i<POY;Poy_i++){
			Init_Loop_3: for(int Pox_i=0;Pox_i<POX;Pox_i++){
				InBuf[Poy_i][index][Pox_i] = 0;
			}
		}
	}
	IF2BUF_Tif:for(int Tif_i=0;Tif_i<TIF;Tif_i++){
		for(int index=0;index<my_ceil(my_ceil(TIY,S),POY);index++){
			for(int S_i=0;S_i<S;S_i++){
				for(int Word1_i=0;Word1_i<WRD_1ROW;Word1_i++){
					Poy_Loop_to_Buf: for(int Poy_i=0;Poy_i<POY;Poy_i++){
						if(index!=0 || Poy_i*S+S_i>=north_pad*ZERO_PAD){
							Pox_Loop_to_Buf: for(int Pox_i=0;Pox_i<POX;Pox_i++){
								if( //Word1_i*POX+Pox_i<TIX-2*ZERO_PAD &&
								Poy_i*S+S_i+index*POY*S<TIY-south_pad*ZERO_PAD &&
										Poy_i*S+S_i+index*POY*S-north_pad*ZERO_PAD>=0){
									InBuf[Poy_i][Word1_i + S_i*WRD_1ROW + index*S*WRD_1ROW
										+Tif_i*ROWS_1MAP*WRD_1ROW][Pox_i]
										= IfMap[Tif_i]
										[Pox_i+Word1_i*POX]
										[IfMap_y_Address+Poy_i*S+S_i+index*POY*S-ZERO_PAD];
								}
							}
						}
					}
				}
			}
		}
	}
}

//void IF2BUF_test(){
//	px_data_t IfMap[Nif][Nix][Niy];
//	// px_data_t ***IfMap;
//	px_data_t InBuf_sw[POY][WRD_INBUF][POX], InBuf_hw[POY][WRD_INBUF][POX];
//	data_bool north_pad, south_pad;
//	for(int Nif_i=0;Nif_i<Nif;Nif_i++){
//		for(int Nix_i=0;Nix_i<Nix;Nix_i++){
//			for(int Niy_i=0;Niy_i<Niy;Niy_i++){
//				IfMap[Nif_i][Nix_i][Niy_i] = 10000*(Nif_i+1) + 100*(Niy_i+1) + Nix_i+1;
//			}
//		}
//	}
//	north_pad = 1;
//	south_pad = 0;
//	IF2BUF(IfMap, InBuf_hw, 0, north_pad, south_pad);
//	IF2BUF_software(IfMap, InBuf_sw, 0, north_pad, south_pad);
//	print_IfMap(IfMap);
//	std::cout << "*****  Software Version  *****" << std::endl;
//	print_InBuf(InBuf_sw);
//	std::cout << "*****  Hardware Version  *****" << std::endl;
//	print_InBuf(InBuf_hw);
//	for(int Poy_i=0;Poy_i<POY;Poy_i++){
//		for(int WRD_INBUF_i=0;WRD_INBUF_i<WRD_INBUF;WRD_INBUF_i++){
//			for(int Pox_i=0;Pox_i<POX;Pox_i++){
//				if(InBuf_hw[Poy_i][WRD_INBUF_i][Pox_i] != InBuf_sw[Poy_i][WRD_INBUF_i][Pox_i]){
//					std::cout << "DETECTED WRONG PIXEL AT TEST BUFFER" << std::endl;
//				}
//			}
//		}
//	}
//}

void print_WtMap(wt_data_t WtMap[Nif][Nof][Nkx][Nky]){
	std::cout << "***  Printing Weight Kernel Maps  ***" << std::endl;
	for(int Nof_i=0;Nof_i<Nof;Nof_i++){
		std::cout << "* Printing IfMap Weight Kernels for " << Nof_i << " OfMap *" << std::endl;
		for(int Nky_i=0;Nky_i<NKY;Nky_i++){
			for(int Nif_i=0;Nif_i<Nif;Nif_i++){
				for(int Nkx_i=0;Nkx_i<NKX;Nkx_i++){
					std::cout << std::setw(6) << WtMap[Nif_i][Nof_i][Nkx_i][Nky_i] << "     ";
				}
				std::cout << "|      ";
			}
			std::cout << " " << std::endl;
		}
	}
}

void print_WtBuf(wt_data_t WtBuf[WRD_WTBUF][POF]){
	std::cout << "***  Printing Weight Buffers  ***" << std::endl;
	for(int Wrd_WtBuf_i=0;Wrd_WtBuf_i<WRD_WTBUF;Wrd_WtBuf_i++){
		for(int Pof_i=0;Pof_i<POF;Pof_i++){
			std::cout << std::setw(6) << WtBuf[Wrd_WtBuf_i][Pof_i] << "     ";
			std::cout << "|      ";
		}
	std::cout << " " << std::endl;
	}
}

void WT2BUF_software(wt_data_t WtMap[Nif][Nof][Nkx][Nky], wt_data_t WtBuf[WRD_WTBUF][POF]){
	// WRD_WTBUF = NKX * NKY * TIF * my_ceil(TOF,POF)
	for(int index1=0;index1<my_ceil(TOF,POF);index1++){
		for(int index2=0;index2<TIF;index2++){
			for(int index3=0;index3<NKY;index3++){
				for(int index4=0;index4<NKX;index4++){
					for(int Pof_i=0;Pof_i<POF;Pof_i++){
#pragma HLS UNROLL
						if(index1*POF+Pof_i<TOF){
							WtBuf[index4+index3*NKX+index2*NKX*NKY+index1*NKX*NKY*TIF][Pof_i]
								= WtMap[index2][index1*POF+Pof_i][index4][index3];
						}
						else{
							WtBuf[index4+index3*NKX+index2*NKX*NKY+index1*NKX*NKY*TIF][Pof_i]
								= 0;
						}
					}
				}
			}
		}
	}
}

void WT2BUF_test(){
	// Declare Variables
	wt_data_t WtMap[Nif][Nof][Nkx][Nky], WtBuf[WRD_WTBUF][POF];
	// Initialize Data
	for(int Nof_i=0;Nof_i<Nof;Nof_i++){
		for(int Nky_i=0;Nky_i<NKY;Nky_i++){
			for(int Nif_i=0;Nif_i<Nif;Nif_i++){
				for(int Nkx_i=0;Nkx_i<NKX;Nkx_i++){
					WtMap[Nif_i][Nof_i][Nkx_i][Nky_i] = (Nof_i+1)*1000000 + (Nif_i+1)*10000
							+ (Nky_i+1)*100 + (Nkx_i+1);
				}
			}
		}
	}
	WT2BUF_software(WtMap, WtBuf);
	print_WtMap(WtMap);
	print_WtBuf(WtBuf);
}

void print_OfMap(px_data_t OfMap[Nof][Nox][Noy]){
	std::cout << "***  Printing Output Feature Map  ***" << std::endl;
	for(int Nof_i=0;Nof_i<Nof;Nof_i++){
		std::cout << Nof_i+1 <<" Map" << std::endl;
		for(int Noy_i=0;Noy_i<Noy;Noy_i++){
			for(int Nox_i=0;Nox_i<Nox;Nox_i++){
				std::cout << std::setw(20) << OfMap[Nof_i][Nox_i][Noy_i] << "     " ;
			}
			std::cout << "|      " << std::endl;
		}

	}
}

void print_OutBuf(px_data_t OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX]){
	std::cout << "***  Printing Output Buffers  ***" << std::endl;
	for(int Wrd_OutBuf_i=0;Wrd_OutBuf_i<WRD_OUTBUF;Wrd_OutBuf_i++){
		for(int OutBuf_Num_i=0;OutBuf_Num_i<OUTBUF_NUM;OutBuf_Num_i++){
			for(int Pox_i=0;Pox_i<POX;Pox_i++){
				std::cout << std::setw(20) << OutBuf[OutBuf_Num_i][Wrd_OutBuf_i][Pox_i] << "     ";
			}
		std::cout << "|      ";
		}
	std::cout << " " << std::endl;
	}
}

// Fix outer parameters
void BUF2OF_software(px_data_t OfMap[Nof][Nox][Noy], px_data_t OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX]){
	// WRD_OUTBUF=my_ceil(TOF, OUTBUF_NUM) * TOY * my_ceil(TOX, POX)
	for(int index3=0;index3<my_ceil(TOF,OUTBUF_NUM);index3++){
		for(int index2=0;index2<TOY;index2++){
			for(int index1=0;index1<my_ceil(TOX,POX);index1++){
				for(int OutBuf_Num_i=0;OutBuf_Num_i<OUTBUF_NUM;OutBuf_Num_i++){
					for(int Pox_i=0;Pox_i<POX;Pox_i++){
						if( index3*OUTBUF_NUM+OutBuf_Num_i<TOF && index1*POX+Pox_i<TOX){
							OfMap[index3*OUTBUF_NUM+OutBuf_Num_i][index1*POX+Pox_i][index2] =
								OutBuf[OutBuf_Num_i]
									[index1+index2*my_ceil(TOX,POX)+index3*my_ceil(TOX,POX)*TOY]
									 [Pox_i];
						}
					}
				}
			}
		}
	}
}

void BUF2OF_test(){
	std::cout << "***  Testing BUF2OF()  ***" << std::endl;
	// Declare Variables
	px_data_t OfMap[Nof][Nox][Noy], OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX];
//	// Initialize Data
//	for(int OutBuf_Num_i=0;OutBuf_Num_i<OUTBUF_NUM;OutBuf_Num_i++){
//		for(int Wrd_OutBuf_i=0;Wrd_OutBuf_i<WRD_OUTBUF;Wrd_OutBuf_i++){
//			for(int Pox_i=0;Pox_i<Nox;Pox_i++){
//				OutBuf[OutBuf_Num_i][Wrd_OutBuf_i][Pox_i] = (OutBuf_Num_i+1)*10000
//						+ (Wrd_OutBuf_i+1)*100 + (Pox_i+1);
//			}
//		}
//	}
	// Alternative Initialization
	// WRD_OUTBUF=my_ceil(TOF, OUTBUF_NUM) * TOY * my_ceil(TOX, POX)
	for(int index1=0;index1<my_ceil(TOF, OUTBUF_NUM);index1++){
		for(int index2=0;index2<TOY;index2++){
			for(int index3=0;index3<my_ceil(TOX, POX);index3++){
				for(int OutBuf_Num_i=0;OutBuf_Num_i<OUTBUF_NUM;OutBuf_Num_i++){
					for(int Pox_i=0;Pox_i<Nox;Pox_i++){
						OutBuf[OutBuf_Num_i]
							[index3+index2*my_ceil(TOX, POX)+index1*my_ceil(TOX, POX)*TOY][Pox_i]
							= (index1*OUTBUF_NUM + OutBuf_Num_i+1)*10000
								+ (index2+1)*100 + (index3*POX+Pox_i+1);
					}
				}
			}
		}
	}

	BUF2OF_software(OfMap, OutBuf);
	print_OutBuf(OutBuf);
	print_OfMap(OfMap);
}

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
void CNN_Layer_golden(px_data_t IfMap[Nif][Nix][Niy], wt_data_t WtMap[Nif][Nof][Nkx][Nky],
		px_data_t OfMap[Nof][Nox][Noy]){
	std::cout << "CNN Parameters:" << std::endl;
    std::cout << "Nif->" << Nif << ", Nof->" << Nof << ", Nix->" << Nix << std::endl;
    std::cout << "Niy->" << Niy << ", Nox->" << Nox << ", Noy->" << Noy << std::endl << std::endl;
	// I dont have parameter flexibility so for full layer I add
    // padding everywhere
	for(int Nof_i=0;Nof_i<Nof;Nof_i++){
		for(int Noy_i=0;Noy_i<Noy;Noy_i++){
			for(int Nox_i=0;Nox_i<Nox;Nox_i++){
				OfMap[Nof_i][Nox_i][Noy_i] = 0;
			}
		}
	}
    px_data_t IfMap_wpad[Nif][Nix+2*ZERO_PAD][Niy+2*ZERO_PAD];
    for(int ni=0;ni<Nif;ni++){ // Loop-4
        for(int y=0;y<Niy+2*ZERO_PAD;y++){ // Loop-3.1
            for(int x=0;x<Nix+2*ZERO_PAD;x++){ // Loop-3.2
            	if(x<ZERO_PAD || x>Nix+ZERO_PAD-1 || y<ZERO_PAD || y>Niy+ZERO_PAD-1){
            		IfMap_wpad[ni][x][y] = 0;
            	}
            	else{
            		IfMap_wpad[ni][x][y] = IfMap[ni][x-ZERO_PAD][y-ZERO_PAD];
            	}
            }
        }
    }
    for(int no=0;no<Nof;no++){ // Loop-4
        for(int y=0;y<Noy;y++){ // Loop-3.1
            for(int x=0;x<Nox;x++){ // Loop-3.2
                for(int ni=0;ni<Nif;ni++){ // Loop-2
                    for(int ky=0;ky<Nky;ky++){ // Loop-1.1
                        for(int kx=0;kx<Nkx;kx++){ // Loop-1.2
                            OfMap[no][x][y] += IfMap_wpad[ni][S*x+kx][S*y+ky]*WtMap[ni][no][kx][ky];
                        }
                    }
                }
            }
        }
    }
}

int Compare_OfMaps(px_data_t OfMap[NOF][NOX][NOY], px_data_t OfMap_golden[NOF][NOX][NOY]){
	float Compared[NOF][NOX][NOY];
	for(int Nof_i=0;Nof_i<NOF;Nof_i++){
		for(int Noy_i=0;Noy_i<NOY;Noy_i++){
			for(int Nox_i=0;Nox_i<NOX;Nox_i++){
//				if( OfMap[Nof_i][Nox_i][Noy_i] > OfMap_golden[Nof_i][Nox_i][Noy_i]){
//					Compared[Nof_i][Nox_i][Noy_i] = OfMap[Nof_i][Nox_i][Noy_i]
//						- OfMap_golden[Nof_i][Nox_i][Noy_i];
//				}
//				else{
//					Compared[Nof_i][Nox_i][Noy_i] = OfMap_golden[Nof_i][Nox_i][Noy_i]
//						- OfMap[Nof_i][Nox_i][Noy_i];
//				}
				Compared[Nof_i][Nox_i][Noy_i] = (OfMap_golden[Nof_i][Nox_i][Noy_i]
					- OfMap[Nof_i][Nox_i][Noy_i])/float(OfMap_golden[Nof_i][Nox_i][Noy_i]);
			}
		}
	}
//	for(int Nof_i=0;Nof_i<Nof;Nof_i++){
//		for(int Noy_i=0;Noy_i<Noy;Noy_i++){
//			for(int Nox_i=0;Nox_i<Nox;Nox_i++){
//				if(Compared[Nof_i][Nox_i][Noy_i]){
//					Compared[Nof_i][Nox_i][Noy_i] = 1;
//				}
//				else{
//					Compared[Nof_i][Nox_i][Noy_i] = 0;
//				}
//			}
//		}
//	}
//	std::cout << "***  OfMap of Boolean Correctness  ***" << std::endl;
//	std::cout << "***  Compared OfMaps  ***" << std::endl;
//	std::cout << "***  Printing Output Feature Map  ***" << std::endl;
//	for(int Nof_i=0;Nof_i<Nof;Nof_i++){
//		std::cout << Nof_i+1 <<" Map" << std::endl;
//		for(int Noy_i=0;Noy_i<Noy;Noy_i++){
//			for(int Nox_i=0;Nox_i<Nox;Nox_i++){
//				std::cout << std::setw(20) << Compared[Nof_i][Nox_i][Noy_i] << "     " ;
//			}
//			std::cout << "|      " << std::endl;
//		}
//	}
	print_ComparedMap(Compared);
	for(int Nof_i=0;Nof_i<Nof;Nof_i++){
		for(int Noy_i=0;Noy_i<Noy;Noy_i++){
			for(int Nox_i=0;Nox_i<Nox;Nox_i++){
				if(Compared[Nof_i][Nox_i][Noy_i]!=0){
					std::cout << "Wrong Results" << std::endl;
					return 1;
				}
			}
		}
	}
	//print_OfMap(Compared);
	return 0;
}

//int BUF_Calc_test(){
//// Is not yet used
//
//	// Declare Variables
//	px_data_t IfMap[Nif][Nix][Niy];
//	px_data_t OfMap[Nof][Nox][Noy], OfMap_Golden_Data[Nof][Nox][Noy];
//	px_data_t InBuf[POY][WRD_INBUF][POX],OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX];
//	wt_data_t WtBuf[WRD_WTBUF][POF], WtMap[Nif][Nof][Nkx][Nky];
//	// Initialize Data
//	for(int Nif_i=0;Nif_i<Nif;Nif_i++){
//		for(int Niy_i=0;Niy_i<Niy;Niy_i++){
//			for(int Nix_i=0;Nix_i<Nix;Nix_i++){
//				IfMap[Nif_i][Nix_i][Niy_i] = 100*(Nif_i+1) + 10*(Niy_i+1) + Nix_i+1;
//			}
//		}
//	}
//	for(int Nif_i=0;Nif_i<Nif;Nif_i++){
//		for(int Nof_i=0;Nof_i<Nof;Nof_i++){
//			for(int Nky_i=0;Nky_i<Nky;Nky_i++){
//				for(int Nkx_i=0;Nkx_i<Nkx;Nkx_i++){
//					WtMap[Nif_i][Nof_i][Nkx_i][Nky_i] = (Nof_i+1)*7 + (Nif_i+1)*5
//							+ (Nky_i+1)*3 + (Nkx_i+1);
//				}
//			}
//		}
//	}
//	IF2BUF_software(IfMap, InBuf, 0, 1, 1);
//	WT2BUF_software(WtMap, WtBuf);
//	print_IfMap(IfMap);
//	print_InBuf(InBuf);
//	print_WtMap(WtMap);
//	print_WtBuf(WtBuf);
//
//	BUF_Calc(InBuf, WtBuf, OutBuf);
//	BUF2OF_software(OfMap, OutBuf);
//	CNN_Layer_golden(IfMap,WtMap, OfMap_Golden_Data);
//
//	print_OutBuf(OutBuf);
//	std::cout << "***  Golden Reference Data  ***" << std::endl;
//	print_OfMap(OfMap_Golden_Data);
//	print_OfMap(OfMap);
//	if(Compare_OfMaps(OfMap, OfMap_Golden_Data)){
//		std::cout << "*****  Output Feature Map results are CORRECT by Testing TILE ONLY  *****" << std::endl;
//		return 1;
//	}
//	else
//		return 0;
//}

int CNN_Layer_test(int verbose){
	// Declare Variables
	px_data_t IfMap[NIF][NIX-2*ZERO_PAD][NIY-2*ZERO_PAD];
	px_data_t OfMap[NOF][NOX][NOY], OfMap_Golden_Data[NOF][NOX][NOY];
	// px_data_t InBuf[POY][WRD_INBUF][POX],OutBuf[OUTBUF_NUM][WRD_OUTBUF][POX];
	// wt_data_t WtBuf[WRD_WTBUF][POF];
	wt_data_t WtMap[NIF][NOF][NKX][NKY];
	// Initialize Data
	for(int Nif_i=0;Nif_i<NIF;Nif_i++){
		for(int Niy_i=0;Niy_i<NIY-2*ZERO_PAD;Niy_i++){
			for(int Nix_i=0;Nix_i<NIX-2*ZERO_PAD;Nix_i++){
				IfMap[Nif_i][Nix_i][Niy_i] = 100*(Nif_i+1) + 10*(Niy_i+1) + Nix_i+1;
			}
		}
	}
	for(int Nif_i=0;Nif_i<NIF;Nif_i++){
		for(int Nof_i=0;Nof_i<NOF;Nof_i++){
			for(int Nky_i=0;Nky_i<NKY;Nky_i++){
				for(int Nkx_i=0;Nkx_i<NKX;Nkx_i++){
					WtMap[Nif_i][Nof_i][Nkx_i][Nky_i] = (Nof_i+1)*7 + (Nif_i+1)*5
							+ (Nky_i+1)*3 + (Nkx_i+1);
				}
			}
		}
	}
	for(int Nof_i=0;Nof_i<NOF;Nof_i++){
		for(int Noy_i=0;Noy_i<NOY;Noy_i++){
			for(int Nox_i=0;Nox_i<NOX;Nox_i++){
				OfMap[Nof_i][Nox_i][Noy_i] = 0;
			}
		}
	}
	CNN_Layer_top(IfMap, WtMap, OfMap);
	CNN_Layer_golden(IfMap,WtMap, OfMap_Golden_Data);
	// print_IfMap(IfMap);
	// print_WtMap(WtMap);
	if(verbose){
		 std::cout << "***  Golden Reference Data  ***" << std::endl;
		 print_OfMap(OfMap_Golden_Data);
		 print_OfMap(OfMap);
	}
	if(Compare_OfMaps(OfMap, OfMap_Golden_Data)){
		return 1;
	}
	else
		std::cout << "*****  Output Feature Map results are CORRECT  *****" << std::endl;
		return 0;
}

int main(){
	// IF2BUF_test();
	// IF2BUF_test();
	// WT2BUF_test();
	// BUF2OF_test();
	// BUF_Calc_test();

	int check, check2;
	check = Print_Check_Parameters();
	// Test 1 Layer from Memory back to Memory
	check2 = CNN_Layer_test(1); // if 1 results are printed
	// Test 1 Tile from Input Buffer to Output Buffer
//	 check2 = BUF_Calc_test();
	// Maybe smaller range tests (Loops of Buffer Traverse, Reuse)
	// ...
	return check + check2;
}

#include "header.h"
#include <iostream>
#include <iomanip>

int Print_Check_Parameters(){
	// Print all Constants/Parameters
	std::cout << "*****  Parameters  *****" << std::endl;
	std::cout << "Stride -> " << S << "  Zero Padding -> " << ZERO_PAD << std::endl;
	std::cout << "N* Parameters:" << std::endl;
	std::cout << "Nkx -> " << NKX << "  Nky -> " << NKY << "  ";
	std::cout << "Nif -> " << NIF << "  Nof -> " << NOF << "  ";
	std::cout << "Nix -> " << NIX << "  Niy -> " << NIY << "  ";
	std::cout << "Nox -> " << NOX << "  Noy -> " << NOY << std::endl;
	std::cout << "T* Parameters:" << std::endl;
	std::cout << "Tif -> " << TIF << "  Tof -> " << TOF << "  ";
	std::cout << "Tix -> " << TIX << "  Tiy -> " << TIY << "  ";
	std::cout << "Tox -> " << TOX << "  Toy -> " << TOY << std::endl;
	std::cout << "P* Parameters:" << std::endl;
	std::cout << "Pif -> " << PIF << "  Pof -> " << POF << "  ";
	std::cout << "Pix -> " << PIX << "  Piy -> " << PIY << "  ";
	std::cout << "Pox -> " << POX << "  Poy -> " << POY << std::endl;
	std::cout << "Buffers Parameters:" << std::endl;
	std::cout << "WORD_1ROW -> " << WRD_1ROW << "  ROWS_1MAP ->";
	std::cout << ROWS_1MAP << "  WRD_INBUF -> " << WRD_INBUF << std::endl;
	std::cout << "WORD_WTBUF -> " << WRD_WTBUF << "  WRD_OUTBUF -> ";
	std::cout << WRD_OUTBUF << std::endl;
	// Check Parameter constraints
	std::cout << "******  Checking Parameter Constraints  ******" << std::endl;
	int check = 0;
	if(NIX<TIX || NIY<TIY || NOX<TOX || NOY<TOY || NIF<TIF){
		std::cout << "T* parameter not smaller than N*." << std::endl;
		check = 1;
	}
	if(TIX<POX || TIY<PIY || TOX<POX || TOY<POY || TIF<PIF){
		std::cout << "P* parameter not smaller than T*." << std::endl;
		check = 1;
	}
	if(OUTBUF_NUM>POF){
		std::cout << "#OutBuf not chosen correctly." << std::endl;
		check = 1;
	}
	if(TOX!=NOX){
		std::cout << "Ifmap line is not all buffered." << std::endl;
		check = 1;
	}
	if(TIF!=NIF || PIF!=1){
		std::cout << "Tif or Pif not chosen correctly." << std::endl;
		check = 1;
	}
	if(my_ceil((NIX-NKX),S) != (NIX-NKX)/S ){
		std::cout << "NIX or NKX not chosen correctly." << std::endl;
		check = 1;
	}
	if(my_ceil((NIY-NKY),S) != (NIY-NKY)/S ){
		std::cout << "NIY or NKY not chosen correctly." << std::endl;
		check = 1;
	}
	std::cout << "******  Parameter Constraints Verified  ******" << std::endl;
	return check;
}

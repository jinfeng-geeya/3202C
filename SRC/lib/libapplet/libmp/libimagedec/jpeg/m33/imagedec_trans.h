#ifndef __IMAGETRANS_HEADER
#define __IMAGETRANS_HEADER

void Init_Scaling_factors(pImagedec_hdl phdl);

void IDCT_Scaling_Cr();

void IDCT_Scaling_Cb();

void PrepareForNextRow(int which_component);

void Rotate270();
void Rotate90();
void Rotate180();

#endif

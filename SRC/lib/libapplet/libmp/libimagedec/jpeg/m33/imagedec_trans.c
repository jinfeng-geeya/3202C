#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libimagedec/imagedec.h>
#include "imagedec_main.h"
#include "imagedec_trans.h"
#include "imagedec_osd.h"
#include "imagedec_acc.h"

extern int g_imagedec_m33_combine_hw_sw_flag;

extern enum IMAGE_ANGLE g_imagedec_angle[IMAGEDEC_MAX_INSTANCE];
extern imagedec_osd_ins g_imagedec_osd_ins;

static int m_trans_ver_no_scale = 0;
static int m_trans_hor_no_scale = 0;
	
void (*IDCT_Scaling_Routine)(pImagedec_hdl phdl, int Y_only);
static void (*IDCT_BilScaling_Routine)(int block_type);

int g_jpeg_dst_stride;
uchar *final_output_buf_Y;
uchar *final_output_buf_C;
//for picture size such as 800x600 in 4:2:0, there are 800/16 = 37.5 MCU lines
//I don't want to waste CPU cycles in detecting this case (this has to be done in inner loop)
//So I reserve some lines to avoid the pointer exceed boundary problem
#if 0
uchar *rotated_output_buf_Y; //frame buffer, I only use 720*576, the same format as hardware
uchar *rotated_output_buf_Cb; //final output is 4:2:0
uchar *rotated_output_buf_Cr;
unsigned long InitYBuf		= (unsigned long)(NULL);
volatile unsigned long InitCbBuf	= (unsigned long)(NULL);
volatile unsigned long InitCrBuf	= (unsigned long)(NULL);
unsigned char * pFBYBuf				= (unsigned char *)(NULL);
unsigned char * pFBCbBuf			= (unsigned char *)(NULL);
unsigned char * pFBCrBuf			= (unsigned char *)(NULL);
unsigned char * pFB2YBuf			= (unsigned char *)(NULL);
unsigned char * pFB2CbBuf			= (unsigned char *)(NULL);
unsigned char * pFB2CrBuf			= (unsigned char *)(NULL);
static int *line_buffer_1; //column buffer for rotation bilinear interpolation
static int *line_buffer_2; //column buffer for rotation bilinear interpolation
#endif

//static uchar TGA_buffer[720*576*3];
static int hor_start_line; //real displayed picture start line (other lines are black)
//static int hor_stop_line;
static int ver_start_line; 
//static int ver_stop_line;

//static int16 hor_partsum_Y[16]; //part sum for horizontal decimation
//static int16 hor_partsum_Cb[16];
//static int16 hor_partsum_Cr[16];
//static int16 hor_bilinear_Y1[16]; //bilinear input sample
//static int16 hor_bilinear_Cb1[16];
//static int16 hor_bilinear_Cr1[16];
static int16 *hor_partsum_Y; //part sum for horizontal decimation
static int16 *hor_partsum_Cb;
static int16 *hor_partsum_Cr;
static int16 *hor_bilinear_Y1; //bilinear input sample
static int16 *hor_bilinear_Cb1;
static int16 *hor_bilinear_Cr1;

//manson: because we accumulate the fraction linear interpolation output, we need 32 bits
//static int ver_partsum_Y[730]; //part sum for vertital decimation
//static int ver_partsum_Cb[370]; //*2 means this store the integer scaling output which may be bigger than 720*576
//static int ver_partsum_Cr[370];
//static int ver_bilinear_Y1[730]; //bilinear input sample
//static int ver_bilinear_Cb1[370];
//static int ver_bilinear_Cr1[370];
static int *ver_partsum_Y; //part sum for vertital decimation
static int *ver_partsum_Cb; //*2 means this store the integer scaling output which may be bigger than 720*576
static int *ver_partsum_Cr;
static int *ver_bilinear_Y1; //bilinear input sample
static int *ver_bilinear_Cb1;
static int *ver_bilinear_Cr1;

static int hor_integer_counter_Y1; //counter for horizontal integer scaling output 
static int hor_integer_counter_Y2; //bugfix002 for 4:2:0
static int hor_fraction_counter_Y1; //counter for horizontal fractional scaling
static int hor_fraction_counter_Y2; //bugfix002 for 4:2:0
static int row_ver_integer_counter_Y; //counter for vertical integer scaling output 
static int row_ver_fraction_counter_Y; //counter for vertical fractional scaling

static int hor_integer_counter_Cb; //counter for horizontal integer scaling output 
static int hor_integer_counter_Cr; //counter for horizontal integer scaling output 
static int row_ver_integer_counter_Cb; //counter for vertical integer scaling output 
static int row_ver_integer_counter_Cr; //counter for vertical integer scaling output 
static int hor_fraction_counter_Cb; //counter for horizontal fractional scaling
static int hor_fraction_counter_Cr; //counter for horizontal fractional scaling
static int row_ver_fraction_counter_Cb; //counter for vertical fractional scaling
static int row_ver_fraction_counter_Cr; //counter for vertical fractional scaling
static int hor_partsum_count_Y1; //bugfix002//counter for samples that has been added to the part sum
static int hor_partsum_count_Y2; //bugfix002 for 4:2:0
static int row_ver_partsum_count_Y; //counter shared by all vertical partsum accumulators
static int hor_partsum_count_Cb; 
static int row_ver_partsum_count_Cb;
static int hor_partsum_count_Cr; 
static int row_ver_partsum_count_Cr;
static int next_row_ver_partsum_count_Y; //this one is used when processing a new block
static int next_row_ver_partsum_count_Cb; //this one is used when processing a new block
static int next_row_ver_partsum_count_Cr; //this one is used when processing a new block

static int hor_final_Y_index_MCU; //MCU final output index
static int ver_final_Y_index_MCU;
static int hor_final_Y_index_block; //block final output index
static int ver_final_Y_index_block;
static int hor_final_Cb_index_block; //block final output index
static int ver_final_Cb_index_block;
static int hor_final_Cr_index_block; //block final output index
static int ver_final_Cr_index_block;
static int hor_final_Cb_index;
static int ver_final_Cb_index;
static int hor_final_Cr_index;
static int ver_final_Cr_index;
static int MCU_block_counter; //counter for decoded blocks in one MCU
static int next_row_ver_integer_counter_Y; //used in processing one block, and copy to MCU counter for next MCU row
static int next_row_ver_fraction_counter_Y;
static int next_row_ver_integer_counter_Cb; //used in processing one block, and copy to MCU counter for next MCU row
static int next_row_ver_fraction_counter_Cb;
static int next_row_ver_integer_counter_Cr; //used in processing one block, and copy to MCU counter for next MCU row
static int next_row_ver_fraction_counter_Cr;
//Manson: conversion from different JPEG CbCr format to 4:2:0 is implemented by seperated Y and CbCr decimation ratio

int TV_ver_lines = 576; //PAL = 576, NTSC = 480, for pixel aspect ratio convertion
int TV_hor_columns = 720; //PAL = 720, NTSC = 720, for pixel aspect ratio convertion

static int hor_scale_ratio_Y_int;  //integer part of horizontal decimation ratio
static int ver_scale_ratio_Y_int;  //integer part of vertical decimation ratio
static int hor_scale_ratio_Y_fraction;  //fraction part of horizontal decimation ratio (1.0~1.999), 1.0 means 720*540 square pixel
static int ver_scale_ratio_Y_fraction;  //fraction part of vertical decimation ratio
static int scale_factor_inverse_2D_Y; //normalize factor (inverse of 2D scaling factor)
static int scale_factor_inverse_2D_Bits; 
static int hor_scale_ratio_CbCr_int;  //integer part of horizontal decimation ratio
static int ver_scale_ratio_CbCr_int;  //integer part of vertical decimation ratio
static int hor_scale_ratio_CbCr_fraction;  //fraction part of horizontal decimation ratio (1.0~1.999)
static int ver_scale_ratio_CbCr_fraction;  //fraction part of vertical decimation ratio
static int scale_factor_inverse_2D_CbCr;
static int block_hor_final_output_counter_Y; //counter for horizontal final output
static int block_ver_final_output_counter_Y; //counter for vertital final output
static int hor_final_output_counter_Cb; //counter for horizontal final output
static int hor_final_output_counter_Cr; //counter for horizontal final output

static int ver_final_output_counter_Cb; //counter for horizontal final output
static int ver_final_output_counter_Cr; //counter for horizontal final output

static int block_first_ver_accumulator_Y1; //index of the first vertical accumulator for the current block
static int block_first_ver_accumulator_Y2; //index of the first vertical accumulator for the current block
static int block_first_ver_accumulator_Cb; //index of the first vertical accumulator for the current block
static int block_first_ver_accumulator_Cr; //index of the first vertical accumulator for the current block
static int row_ver_final_output_counter_Y; //counter for vertital final output
static int row_ver_final_output_counter_Cb; //counter for vertital final output
static int row_ver_final_output_counter_Cr; //counter for vertital final output

static int row_hor_final_output_counter_Cb; //counter for vertital final output
static int row_hor_final_output_counter_Cr; //counter for vertital final output

//static int temp_IDCT_buf[64]; //IDCT input, 
//static int temp_IDCT_buf2[128]; //IDCT output, for 4:1:1 we may have to do horizontal copying
extern  int *temp_IDCT_buf; //IDCT input, 
extern int *temp_IDCT_buf2; //IDCT output, for 4:1:1 we may have to do horizontal copying

static int MCU_output_ver_count;//bugfix002, MCU output vertical counter


static int MCU_output_hor_count;

static int MCU_hor_partsum_first; //bugfix002, = 0 or 8. indicating which hor 8 partsum to use
static int debug_counter = 0;
static int debug_counter2 = 0;
static int IDCT_mode_Y; //0 - 8x8, 1 - 4x4, 2 - 2x2, 3 - 1x1
static int IDCT_mode_CbCr; 
//0 - 8x8, 1 - 4x4, 2 - 2x2, 3 - 1x1, 
//8 - 8x4, 9 - 4x2, 10 - 2x1
//         17- 8x2, 18- 4x1
static int flag_CbCr_Hor_copy; //for 4:1:1 if the picture size is too small, we need horizontal scaling up for CbCr
static int idct_row_num_Y; //8/4/2/1
static int idct_row_num_Y1; //if the picture size is not integer times of MCU size, discard some rows
static int idct_row_num_Y2; //change009 for MCU that has two Y blocks vertically, we need two row number value
static int idct_col_num_Y; //8/4/2/1
static int idct_row_num_CbCr; 
static int idct_col_num_CbCr;//change009
static int flag_hor_hiding_line_added;
static int flag_hor_hiding_line_added_Y;
static int flag_hor_hiding_line_added_Cb;
static int flag_hor_hiding_line_added_Cr;
static int flag_need_scaling_up; //we use hardware scaling engine to do scaling up
static int decoded_picture_height; //picture size including added black lines, <= 736xTV_ver_lines

static int decoded_picture_real_height_Y; //for computing extra lines to discard. = decoded_picture_height - black lines
static int decoded_picture_real_height_CbCr; //change009 for computing extra lines to discard. = decoded_picture_height - black lines
static int decoded_picture_width;
static int decoded_picture_height_counter_Y; //counter the height of the already decoded picture
static int decoded_picture_height_counter_Cb; //In progressive JPEG, picture is processed component by component
static int decoded_picture_height_counter_Cr; 
static int MCU_ver_Y_block_num; //vertically number of Y blocks in a MCU
static int hor_final_Y_index_MCU_tophalf; //change014 for last MCU line in not integer MCU picture size

static int MCU_hor_Y_block_num; 
static int ver_final_Y_index_MCU_tophalf; 

static int decoded_picture_real_width_Y; //for computing extra lines to discard. = decoded_picture_height - black lines
static int decoded_picture_real_width_CbCr; //change009 for computing extra lines to discard. = decoded_picture_height - black lines
static int decoded_picture_width_counter_Y; //counter the height of the already decoded picture
static int decoded_picture_width_counter_Cb; //In progressive JPEG, picture is processed component by component
static int decoded_picture_width_counter_Cr; 
static int idct_col_num_Y_ForMost; //change014, column number for most MCU blocks
static int idct_col_num_Y_block1; //change014, column number for the last MCU in one line
static int idct_col_num_Y_block2;
static int idct_col_num_Y_block3;
static int idct_col_num_Y_block4;
static int idct_col_num_CbCr_ForMost; //change014, column number for most MCU blocks
static int idct_col_num_CbCr_ForLast; // change014, column number for the last MCU in one line
static int MCU_hor_Y_block_num; // change014, horizontally number of Y blocks in a MCU
static int flag_is_last_hor_MCU; //flag indicating it is the last MCU in a horizontal MCU line
static int first_ver_final_output_counter_Y; //if the picture is very large, there is a special case that block 2 doesn't have output, save block 1 output counter for this case
static int third_ver_final_output_counter_Y; //if the picture is very large, for 4:2:0 there is a special case that block 4 doesn't have output, save block 3 output counter for this case
static int first_hor_final_output_counter_Y; 
static int third_hor_final_output_counter_Y;

static int flag_very_large_picture; //change016 indicating picture size is too large that Y scaling need handling scale factor >= 2.0
static int ver_integer_counter_Y1; //change016
static int ver_integer_counter_Y2; //for support 4:2:0, block 1-3 share one counter, block 2-4 share another
static int ver_fraction_counter_Y1; 
static int ver_fraction_counter_Y2; 
static int block_ver_integer_counter_Y; //change016 for supporting 4:2:0
static int block_ver_fraction_counter_Y;
static int ver_partsum_count_Y1; //change018
static int ver_partsum_count_Y2; //for support 4:2:0, block 1-3 share one counter, block 2-4 share another
static int block_ver_partsum_count_Y; //change018, move from a local variable
static int max_MCU_output_ver_count_Y; //change018, if the picture size is very large, we don't know which MCU has vertical output, so 

static int max_MCU_output_hor_count_Y;

static int data_error_counter=0; //for some unknown reason reading or writing uncacheable data my result in error, 
//avoid the pointer overflow problem when picture size is not integer times of MCU size
//end of manson defined variables

//frank add
static int	current_hor_start_line;
static int	current_hor_stop_line;
static int	current_ver_start_line;
static int	current_ver_stop_line;
static int scale_precision,scale_precisionX2;		
static int scale_bits_field;
static int scale_fra_total_value;

//#define SCALE_CODE_OPTIMIZATION

static void IDCT_BilinearScaling_ORI(int block_type)
{
//This is a simplified version from EnhancedBilinearScaling()
//This function can only process 1.0 < scale_factor < 2.0
//So there is no integer scaling factor. So the program run faster than Enhanced Bilinear scaling
    //manson:  do scaling for this block
	int ver_counter;
	int hor_counter;
	int temp_hor_partsum_count = 0;
	int block_hor_integer_counter = 0;
	int temp_hor_fraction_counter = 0;
	int temp_final_output;
	int flag_final_output_next_row; //indicating final output point will move to next row
	int *pIDCT_Output; //bugfix001 use 32 bits for IDCT buffer

	uchar *pOutput_row_first_sample;
	uchar *pOutput_current_sample;

	int *pFirst_ver_accumulator;   //32 bits
	int *pFirst_ver_bilinear_sample; //point to the first vertical bilinear input sample to be processed
	int *pCurrent_ver_accumulator = NULL; //used for vertical accumulating 
	int *pCurrent_ver_accumulator2; //use for vertical accumulator output
	int *pCurrent_ver_blinear_sample = NULL; //used for vertical bilinear interpolation 
	int temp_output; 
	int fraction;
	int temp_ver_index; //bugfix002
	int i;

	//if the horizontal scaling factor is too large, for some MCU the veritcal parameters
	//below is not changed at all while for other MCUs it is change
	//it is necessary to record the vertical parameter change for the next row
	int vertical_para_change = 0; //indicating vertical parameter is changed
	int block_ver_partsum_count_Cb; //this one is used when processing a new block
	int block_ver_partsum_count_Cr; //this one is used when processing a new block
	int block_ver_integer_counter_Cb; //used in processing one block, and copy to MCU counter for next MCU row
	int block_ver_fraction_counter_Cb;
	int block_ver_integer_counter_Cr; //used in processing one block, and copy to MCU counter for next MCU row
	int block_ver_fraction_counter_Cr;
	int block_ver_final_output_counter_Cb; //counter for vertital final output
    	int block_ver_final_output_counter_Cr; //counter for vertital final output

	//int block_curn_ver_accumulator_Y; //change016
	//int block_curn_ver_accumulator_Y2; //change016

	int current_sample_pos=0;
	int x_pos,y_pos,x_mb_pos, y_mb_pos;
	

	//hor_final_output_counter and ver_final_output_counter will be used to update output address
	pIDCT_Output = temp_IDCT_buf2;

	if(block_type==0){ //current block is Y
		//manson: now we do transform for this block

		if(!g_imagedec_m33_combine_hw_sw_flag)
		{
			switch(IDCT_mode_Y){
				case 0:
					idct(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 1:
					idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 2:
					idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 3:
					idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
			}
		}
		
		//change016
		//block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		//block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		//block_ver_partsum_count_Y = row_ver_partsum_count_Y; //change 018,vertical counters will be updated when preparing next row of MCU
		block_hor_final_output_counter_Y = 0;
		block_ver_final_output_counter_Y = 0;
#ifdef SCALE_CODE_OPTIMIZATION
		x_pos = hor_final_Y_index_block;
		y_pos = ver_final_Y_index_block;
#endif
		//pointer to first row's first sample 
		pOutput_row_first_sample = final_output_buf_Y;
		//pointer to first vertical accumulator
		if(MCU_hor_partsum_first==0){ //for 4:2:0 which has 4 Y blocks
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y1]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y1];
		}
		else{
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y2]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y2];
		}
		
		flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
		for(ver_counter=0;ver_counter<idct_row_num_Y;ver_counter++){ //vertical counter
			pCurrent_ver_accumulator = pFirst_ver_accumulator;
			flag_final_output_next_row = 0;
				
			if(MCU_hor_partsum_first==0){//bugfix002
				block_hor_integer_counter = hor_integer_counter_Y1;
				temp_hor_partsum_count = hor_partsum_count_Y1; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y1; //counter for horizontal fractional scaling
			}
			else{
				block_hor_integer_counter = hor_integer_counter_Y2;
				temp_hor_partsum_count = hor_partsum_count_Y2; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y2; //counter for horizontal fractional scaling
			}
			temp_ver_index = MCU_hor_partsum_first+ver_counter;//bugfix002

			if(1 == m_trans_ver_no_scale)
				pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
				
			for(hor_counter=0;hor_counter<idct_col_num_Y;hor_counter++){ //horizontal counter
				hor_partsum_Y[temp_ver_index] += *pIDCT_Output; //bugfix002//sum for horizontal scaling
				
				if(1 == m_trans_hor_no_scale)
					hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];
					
				pIDCT_Output++;
				temp_hor_partsum_count++; 
				if(temp_hor_partsum_count>=hor_scale_ratio_Y_int){ //horizontal ready to output one sample
					block_hor_integer_counter++;
					if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
						hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];//bugfix002
					}
					else{ //ready to do horizontal linear interpolation
						fraction = temp_hor_fraction_counter & scale_bits_field;
						temp_output = hor_bilinear_Y1[temp_ver_index]*(scale_fra_total_value-fraction) + hor_partsum_Y[temp_ver_index]*fraction;//bugfix002
						//add to vertical accumulator
						*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling

						if(1 == m_trans_ver_no_scale)
							*pCurrent_ver_blinear_sample++ = *pCurrent_ver_accumulator;
							
						pCurrent_ver_accumulator++; //point to next vertical sum						
						temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_Y_fraction;
						//update hor_bilinear_Y1 since fractional scale factor >= 1.0
						hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];//bugfix002
					}
					hor_partsum_Y[temp_ver_index] = 0; //bugfix002//clear horizontal sum since it has been added to vertical sum
					temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
				}
			}
			pIDCT_Output += 8 - hor_counter;
			if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
				block_ver_partsum_count_Y++;
				vertical_para_change = 1;
				current_sample_pos = hor_final_Y_index_block;
				if(block_ver_partsum_count_Y >= ver_scale_ratio_Y_int){//vertical ready to output one sample
					block_hor_final_output_counter_Y = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
					pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
					pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
#ifdef SCALE_CODE_OPTIMIZATION
					
#else
					pOutput_current_sample = pOutput_row_first_sample;
#endif
					block_ver_integer_counter_Y++;
					if(block_ver_integer_counter_Y <= (block_ver_fraction_counter_Y>>scale_precision)){//not ready to do interplation yet
						while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
							*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							pCurrent_ver_accumulator2++;
							pCurrent_ver_blinear_sample++;
						}
					}
					else{//ready to do vertical linear interpolation
#ifdef SCALE_CODE_OPTIMIZATION
						pOutput_current_sample = pOutput_row_first_sample + (y_pos & 0xFFF0) * (g_jpeg_dst_stride<<4) + ((y_pos & 0x0F) <<4) + \
							((x_pos & 0xFFF0) <<4) + (x_pos & 0x0F);
#endif
						fraction = block_ver_fraction_counter_Y & scale_bits_field;				
						while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
							temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
							temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
							if(0 == scale_factor_inverse_2D_Bits)
							{
								temp_final_output = temp_output / scale_factor_inverse_2D_Y;
							}
							else
							{
								temp_final_output = temp_output * scale_factor_inverse_2D_Y;
								temp_final_output >>= scale_factor_inverse_2D_Bits;
							}
#ifdef SCALE_CODE_OPTIMIZATION			


							*pOutput_current_sample++ = (uchar)clamp(temp_final_output);
							if(0xF == (x_pos&0xF))
								pOutput_current_sample += 240;
							x_pos++;
#else
							x_mb_pos = (current_sample_pos>>4);
							x_pos = (current_sample_pos - ((current_sample_pos>>4)<<4));
							y_mb_pos = ((block_ver_final_output_counter_Y+ver_final_Y_index_block)>>4);
							y_pos = (block_ver_final_output_counter_Y+ver_final_Y_index_block) -(((block_ver_final_output_counter_Y+ver_final_Y_index_block)>>4)<<4);
							pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + x_pos] = (uchar)clamp(temp_final_output);	
							current_sample_pos++;
#endif							
							//update ver_bilinear_Y1 since fractional scale factor >= 1.0
							*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							pCurrent_ver_accumulator2++;
							pCurrent_ver_blinear_sample++;
						}
						block_ver_fraction_counter_Y = block_ver_fraction_counter_Y + ver_scale_ratio_Y_fraction;
						flag_final_output_next_row = 1;
					}
					block_ver_partsum_count_Y = 0; //clear vertical partsum counter
				}
			}
			if(flag_final_output_next_row!=0){ //indicating final output will move to next row
			//	pOutput_row_first_sample += Final_Output_Buf_Stride_Y; //final output move to next row
				block_ver_final_output_counter_Y++;
#ifdef SCALE_CODE_OPTIMIZATION
				y_pos++;
#endif
			}
		}
		if(MCU_hor_partsum_first==0){//bugfix002
			hor_partsum_count_Y1 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y1 = temp_hor_fraction_counter;
			hor_integer_counter_Y1 = block_hor_integer_counter;
		}
		else{
			hor_partsum_count_Y2 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y2 = temp_hor_fraction_counter;
			hor_integer_counter_Y2 = block_hor_integer_counter;
		}
		if(vertical_para_change != 0){
			next_row_ver_partsum_count_Y = block_ver_partsum_count_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_integer_counter_Y = block_ver_integer_counter_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_fraction_counter_Y = block_ver_fraction_counter_Y; //for 4:2:0, the last Y block para is used
			if(MCU_hor_partsum_first==0)
				block_first_ver_accumulator_Y1 = block_first_ver_accumulator_Y1 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			else
				block_first_ver_accumulator_Y2 = block_first_ver_accumulator_Y2 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
		}
		//block_ver_final_output_counter_Y and block_hor_final_output_counter_Y will be use to update output address for next block

#if (0)		
		//manson: now we do transform for this block
		
		switch(IDCT_mode_Y){
		case 0:
			idct(temp_IDCT_buf, temp_IDCT_buf2);
			break;
		case 1:
			idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
			break;
		case 2:
			idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
			break;
		case 3:
			idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
			break;
		}
		
		//change016
		//block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		//block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		//block_ver_partsum_count_Y = row_ver_partsum_count_Y;//change018 vertical counters will be updated when preparing next row of MCU
		block_hor_final_output_counter_Y = 0;
		block_ver_final_output_counter_Y = 0;
		
		//pointer to first row's first sample 
		pOutput_row_first_sample = final_output_buf_Y;
	/*	if(cur_decoding_mcu_row==2)
			libc_printf("ver_index:%d hor_index:%d\n",ver_final_Y_index_block,hor_final_Y_index_block);
		pOutput_row_first_sample = final_output_buf_Y+(ver_final_Y_index_block/16)*45*16*16 + 720*16*6;
//		pOutput_row_first_sample = &final_output_buf_Y[(ver_final_Y_index_block*output_stride) + hor_final_Y_index_block];
		if(cur_decoding_mcu_row==2)
			libc_printf("pOutput:%x\n",pOutput_row_first_sample);
		pOutput_current_line = pOutput_row_first_sample;*/
		//pointer to first vertical accumulator
		if(MCU_hor_partsum_first==0){ //for 4:2:0 which has 4 Y blocks
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y1]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y1];
			block_curn_ver_accumulator_Y = block_first_ver_accumulator_Y1;
		}
		else{
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y2]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y2];
			block_curn_ver_accumulator_Y = block_first_ver_accumulator_Y2;
		}
		
		flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
		for(ver_counter=0;ver_counter<idct_row_num_Y;ver_counter++){ //vertical counter

		//	if(cur_decoding_mcu_row==2)
		//		libc_printf("ver_index:%d hor_index:%d\n",ver_final_Y_index_block,hor_final_Y_index_block);
		//	pOutput_row_first_sample = final_output_buf_Y+((ver_final_Y_index_block+block_ver_final_output_counter_Y)/16)*45*16*16 + 720*16*6;
	//		pOutput_row_first_sample = &final_output_buf_Y[(ver_final_Y_index_block*output_stride) + hor_final_Y_index_block];
	//		if(cur_decoding_mcu_row==2)
		//		libc_printf("pOutput:%x\n",pOutput_row_first_sample);
			//change016
			if(MCU_hor_partsum_first==0){
				block_curn_ver_accumulator_Y = block_first_ver_accumulator_Y1;
				block_curn_ver_accumulator_Y2 = block_first_ver_accumulator_Y1;
			}
			else{
				block_curn_ver_accumulator_Y = block_first_ver_accumulator_Y2;
				block_curn_ver_accumulator_Y2 = block_first_ver_accumulator_Y2;
			}


			pCurrent_ver_accumulator = pFirst_ver_accumulator;
			flag_final_output_next_row = 0;
			
			if(MCU_hor_partsum_first==0){//bugfix002
				block_hor_integer_counter = hor_integer_counter_Y1;
				temp_hor_partsum_count = hor_partsum_count_Y1; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y1; //counter for horizontal fractional scaling
			}
			else{
				block_hor_integer_counter = hor_integer_counter_Y2;
				temp_hor_partsum_count = hor_partsum_count_Y2; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y2; //counter for horizontal fractional scaling
			}
			temp_ver_index = MCU_hor_partsum_first+ver_counter;//bugfix002
			for(hor_counter=0;hor_counter<idct_col_num_Y;hor_counter++){ //horizontal counter
				block_hor_integer_counter++;
				if(block_hor_integer_counter<=(temp_hor_fraction_counter>>8)){//not ready for linear interpolation
					hor_bilinear_Y1[temp_ver_index] = *pIDCT_Output;//bugfix002
				//	libc_printf("orgval:%x\n",hor_bilinear_Y1[temp_ver_index]);
					
				}
				else{ //ready to do horizontal linear interpolation
					fraction = temp_hor_fraction_counter & 0xff;
				//	libc_printf("val1:%x val2:%x fraction:%x\n",hor_bilinear_Y1[temp_ver_index],(*pIDCT_Output),fraction);
					temp_output = hor_bilinear_Y1[temp_ver_index]*(0x100-fraction) + (*pIDCT_Output)*fraction;//bugfix002
					//accumulator is always 0
					//change016
					ver_partsum_Y[block_curn_ver_accumulator_Y] = temp_output; 
				//	libc_printf("<%d>iptval:%x\n",block_curn_ver_accumulator_Y,ver_partsum_Y[block_curn_ver_accumulator_Y]);
					block_curn_ver_accumulator_Y++;
					//*pCurrent_ver_accumulator = temp_output; //sum for vertical scaling
					//pCurrent_ver_accumulator++; //point to next vertical sum
					temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_Y_fraction;
					//update hor_bilinear_Y1 since fractional scale factor >= 1.0
					hor_bilinear_Y1[temp_ver_index] = *pIDCT_Output;//bugfix002
					
				}
				pIDCT_Output++;
			}
			pIDCT_Output += 8 - hor_counter;
		//	libc_printf("block_curn_ver_accumulator_Y:%d block_curn_ver_accumulator_Y2:%d \n",block_curn_ver_accumulator_Y,block_curn_ver_accumulator_Y2);
			//if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
			if(block_curn_ver_accumulator_Y!=block_curn_ver_accumulator_Y2){
				//block_ver_partsum_count_Y++;
				vertical_para_change = 1; //this is needed for the 1x1 IDCT case
				//if(block_ver_partsum_count_Y >= ver_scale_ratio_Y_int){//vertical ready to output one sample
					//block_hor_final_output_counter_Y = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
					block_hor_final_output_counter_Y = block_curn_ver_accumulator_Y - block_curn_ver_accumulator_Y2; //unfortunately we have to compute it multiple times
					
					pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
					//libc_printf("block_hor_final_output_counter_Y:%d \n",block_hor_final_output_counter_Y);
					pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
					current_sample_pos = hor_final_Y_index_block;


				//	libc_printf("pOutput_current_line:%x current_sample_pos:%d\n",pOutput_current_line,current_sample_pos);
					//pOutput_current_sample = pOutput_current_line;

					pOutput_current_sample = pOutput_row_first_sample;
				
					block_ver_integer_counter_Y++;
				//	libc_printf("block_ver_integer_counter_Y:%x block_ver_fraction_counter_Y:%x\n",block_ver_integer_counter_Y,block_ver_fraction_counter_Y);
					if(block_ver_integer_counter_Y<=(block_ver_fraction_counter_Y>>8)){//not ready to do interplation yet
						//while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
						//	*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							//*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
						//	pCurrent_ver_accumulator2++;
						//	pCurrent_ver_blinear_sample++;
						//}
						while(block_curn_ver_accumulator_Y2!=block_curn_ver_accumulator_Y){
							ver_bilinear_Y1[block_curn_ver_accumulator_Y2] = ver_partsum_Y[block_curn_ver_accumulator_Y2];
							block_curn_ver_accumulator_Y2++;
						}
					}
					else{//ready to do vertical linear interpolation
						fraction = block_ver_fraction_counter_Y & 0xff;
					//	if(cur_decoding_mcu_row==2)
					//	libc_printf("pOutput_current_sample:%x\n",pOutput_current_sample);
						//while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
						while(block_curn_ver_accumulator_Y2!=block_curn_ver_accumulator_Y){
							//temp_output = (*pCurrent_ver_blinear_sample)*(0x100-fraction) + (*pCurrent_ver_accumulator2)*fraction;
							temp_output = ver_bilinear_Y1[block_curn_ver_accumulator_Y2]*(0x100-fraction) + ver_partsum_Y[block_curn_ver_accumulator_Y2]*fraction;
							
							temp_output = temp_output>>16;//>> 16 to discard the fractional bits
							temp_final_output = temp_output * scale_factor_inverse_2D_Y;
							temp_final_output >>= 15;

							x_mb_pos = (current_sample_pos>>4);
							x_pos = (current_sample_pos - ((current_sample_pos>>4)<<4));
							y_mb_pos = ((block_ver_final_output_counter_Y+ver_final_Y_index_block)>>4);

							y_pos = (block_ver_final_output_counter_Y+ver_final_Y_index_block) - (((block_ver_final_output_counter_Y+ver_final_Y_index_block)>>4)<<4);
						//	y_pos = (block_ver_final_output_counter_Y+ver_final_Y_index_block)%16;
						//	if(cur_decoding_mcu_row==2)
						//	libc_printf("pos = %x\n",(x_mb_pos<<8)+((block_ver_final_output_counter_Y+ver_final_Y_index_block)<<4)+x_pos);

						//	pOutput_current_sample[(x_mb_pos<<8)+((block_ver_final_output_counter_Y+ver_final_Y_index_block)<<4)+x_pos] = (uchar)clamp(temp_final_output); 
							pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + x_pos] = (uchar)clamp(temp_final_output); 
							//if((*pOutput_current_sample!=0)&&(MCU_hor_partsum_first==0))
							//update ver_bilinear_Y1 since fractional scale factor >= 1.0
							//*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
						//	libc_printf("sample:%x\n",(uchar)clamp(temp_final_output) );
							ver_bilinear_Y1[block_curn_ver_accumulator_Y2] = ver_partsum_Y[block_curn_ver_accumulator_Y2];
							block_curn_ver_accumulator_Y2++;
							//*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//pOutput_current_sample++;
							current_sample_pos++;
							pCurrent_ver_accumulator2++;
						//	pCurrent_ver_blinear_sample++;
						//	current_pixel_num[current_line]++;
							
						}
						block_ver_fraction_counter_Y = block_ver_fraction_counter_Y + ver_scale_ratio_Y_fraction;
						flag_final_output_next_row = 1;
					}
					//block_ver_partsum_count_Y = 0; //clear vertical partsum counter
				//}
			}
			if(flag_final_output_next_row!=0){ //indicating final output will move to next row
			//	pOutput_row_first_sample += Final_Output_Buf_Stride_Y; //final output move to next row
				block_ver_final_output_counter_Y++;
			//	if(block_ver_final_output_counter_Y==16)
			}
		}
		if(MCU_hor_partsum_first==0){
			block_first_ver_accumulator_Y1 = block_curn_ver_accumulator_Y;
		}
		else{
			block_first_ver_accumulator_Y2 = block_curn_ver_accumulator_Y;
		}

		if(MCU_hor_partsum_first==0){//bugfix002
			hor_partsum_count_Y1 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y1 = temp_hor_fraction_counter;
			hor_integer_counter_Y1 = block_hor_integer_counter;
		}
		else{
			hor_partsum_count_Y2 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y2 = temp_hor_fraction_counter;
			hor_integer_counter_Y2 = block_hor_integer_counter;
		}
		if(vertical_para_change != 0){
			next_row_ver_partsum_count_Y = block_ver_partsum_count_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_integer_counter_Y = block_ver_integer_counter_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_fraction_counter_Y = block_ver_fraction_counter_Y; //for 4:2:0, the last Y block para is used
			if(MCU_hor_partsum_first==0)
				block_first_ver_accumulator_Y1 = block_first_ver_accumulator_Y1 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			else
				block_first_ver_accumulator_Y2 = block_first_ver_accumulator_Y2 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
		}
		//block_ver_final_output_counter_Y and block_hor_final_output_counter_Y will be use to update output address for next block
#endif		
	}
	else{ //Cb or Cr
	
		//manson: now we do transform for this block
		if(!g_imagedec_m33_combine_hw_sw_flag)
		{		
			switch(IDCT_mode_CbCr){
				case 0:
					idct(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 1:
					idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 2:
					idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 3:
					idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 8:
					idct8x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 9:
					idct4x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 10:
					idct2x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 17:
					idct8x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 18:
					idct4x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
			}
		}
		
	//	libc_printf("IDCT_mode_CbCr:%d\n",IDCT_mode_CbCr);SDBBP();
		if(flag_CbCr_Hor_copy){
			for(i=127;i>=0;i--){
				temp_IDCT_buf2[i] = temp_IDCT_buf2[i>>1];
			}
		}
		if(block_type==1){ //current block is Cb
			//manson: because CbCr may have larger scale factor, 
			//so CbCr still use enhanced bilinear scaling
			block_ver_integer_counter_Cb = row_ver_integer_counter_Cb; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Cb = row_ver_fraction_counter_Cb;
			block_ver_partsum_count_Cb = row_ver_partsum_count_Cb;
			hor_final_output_counter_Cb = 0;
			block_ver_final_output_counter_Cb = 0;
			//pointer to first row's first sample 
			//pOutput_row_first_sample = &final_output_buf_Cb[(ver_final_Cb_index_block*Final_Output_Buf_Stride_CbCr) + hor_final_Cb_index_block];
			pOutput_row_first_sample = final_output_buf_C;
			//pointer to first vertical accumulator
			//manson: if scale factor is too large, some MCU rows may not have final output
			//so we can not take the final output index as the vertical accumulator index (although for small scale factor, they are equivalent)
			pFirst_ver_accumulator = &ver_partsum_Cb[block_first_ver_accumulator_Cb];
			pFirst_ver_bilinear_sample = &ver_bilinear_Cb1[block_first_ver_accumulator_Cb];
			flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
			for(ver_counter=0;ver_counter<idct_row_num_CbCr;ver_counter++){ //vertical counter
				pCurrent_ver_accumulator = pFirst_ver_accumulator;
				flag_final_output_next_row = 0;
				block_hor_integer_counter = hor_integer_counter_Cb;
				temp_hor_partsum_count = hor_partsum_count_Cb; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Cb; //counter for horizontal fractional scaling

				if(1 == m_trans_ver_no_scale)
					pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;	
							
				for(hor_counter=0;hor_counter<idct_col_num_CbCr;hor_counter++){ //horizontal counter
					hor_partsum_Cb[ver_counter] += *pIDCT_Output; //sum for horizontal scaling

					if(1 == m_trans_hor_no_scale)
						hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];				
					
					pIDCT_Output++;
					temp_hor_partsum_count++; 
					if(temp_hor_partsum_count>=hor_scale_ratio_CbCr_int){ //horizontal ready to output one sample
						block_hor_integer_counter++;
						if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
							hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];
						}
						else{ //ready to do horizontal linear interpolation
							fraction = temp_hor_fraction_counter & scale_bits_field;
							temp_output = hor_bilinear_Cb1[ver_counter]*(scale_fra_total_value-fraction) + hor_partsum_Cb[ver_counter]*fraction;
							//add to vertical accumulator
							*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling

							if(1 == m_trans_ver_no_scale)
								*pCurrent_ver_blinear_sample++ = *pCurrent_ver_accumulator;
						
							pCurrent_ver_accumulator++; //point to next vertical sum
							temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_CbCr_fraction;
							//update hor_bilinear_Cb1 since fractional scale factor >= 1.0
							hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];
						}
						hor_partsum_Cb[ver_counter] = 0; //clear horizontal sum since it has been added to vertical sum
						temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
					}
				}
				if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
					block_ver_partsum_count_Cb++; 
					vertical_para_change = 1;
					current_sample_pos = hor_final_Cb_index_block;
					if(block_ver_partsum_count_Cb >= ver_scale_ratio_CbCr_int){//vertical ready to output one sample
						hor_final_output_counter_Cb = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
						pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
						pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
						pOutput_current_sample = pOutput_row_first_sample;
						block_ver_integer_counter_Cb++;
						if(block_ver_integer_counter_Cb<=(block_ver_fraction_counter_Cb>>scale_precision)){//not ready to do interplation yet
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
						}	
						else{//ready to do vertical linear interpolation						
							fraction = block_ver_fraction_counter_Cb & scale_bits_field;
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
								temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits

								if(0 == scale_factor_inverse_2D_Bits)
								{
									temp_final_output = temp_output / scale_factor_inverse_2D_CbCr;
								}
								else
								{
									temp_final_output = temp_output * scale_factor_inverse_2D_CbCr;
									temp_final_output >>= scale_factor_inverse_2D_Bits;
								}

								x_mb_pos = (current_sample_pos>>3);
								x_pos = (current_sample_pos - ((current_sample_pos>>3)<<3));
								y_mb_pos = ((block_ver_final_output_counter_Cb+ver_final_Cb_index_block)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = (block_ver_final_output_counter_Cb+ver_final_Cb_index_block) - (((block_ver_final_output_counter_Cb+ver_final_Cb_index_block)>>4)<<4);
							//	libc_printf("x_:%d y_:%d\n",current_sample_pos,(block_ver_final_output_counter_Cb+ver_final_Cb_index_block));
							//	libc_printf("pos1:%x\n", y_mb_pos*45*16*16 + (x_mb_pos<<8) + y_pos*16 + x_pos*2 + 1);
								pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8)+ (x_mb_pos<<8) + (y_pos<<4) + (x_pos<<1) + 1] = (uchar)clamp(temp_final_output); 


								//update ver_bilinear_Cb1 since fractional scale factor >= 1.0
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//	pOutput_current_sample++;
								current_sample_pos++;
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
							block_ver_fraction_counter_Cb = block_ver_fraction_counter_Cb + ver_scale_ratio_CbCr_fraction;
							flag_final_output_next_row = 1;
						}
						block_ver_partsum_count_Cb = 0; //clear vertical partsum counter
					}
				}
				if(flag_final_output_next_row!=0){ //indicating final output will move to next row
				//	pOutput_row_first_sample += Final_Output_Buf_Stride_CbCr; //final output move to next row
					block_ver_final_output_counter_Cb++;
				}
			}
			hor_partsum_count_Cb = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Cb = temp_hor_fraction_counter;
			hor_integer_counter_Cb = block_hor_integer_counter;
			if(vertical_para_change != 0){
				next_row_ver_partsum_count_Cb = block_ver_partsum_count_Cb;
				next_row_ver_integer_counter_Cb=block_ver_integer_counter_Cb;
				next_row_ver_fraction_counter_Cb = block_ver_fraction_counter_Cb;
				row_ver_final_output_counter_Cb = block_ver_final_output_counter_Cb; //counter for vertital final output
				block_first_ver_accumulator_Cb = block_first_ver_accumulator_Cb + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			}
			//if(block_ver_partsum_count_Cb != row_ver_partsum_count_Cb)
			//	next_row_ver_partsum_count_Cb = block_ver_partsum_count_Cb;
			//block_ver_final_output_counter_Cb and hor_final_output_counter_Cb will be use to update output address for next block
		}
		else{//current block is Cr
			//for debug
			block_ver_integer_counter_Cr = row_ver_integer_counter_Cr;//vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Cr = row_ver_fraction_counter_Cr;
			block_ver_partsum_count_Cr = row_ver_partsum_count_Cr;
			hor_final_output_counter_Cr = 0;
			block_ver_final_output_counter_Cr = 0;
			//pointer to first row's first sample 
		//	pOutput_row_first_sample = &final_output_buf_Cr[(ver_final_Cr_index_block*Final_Output_Buf_Stride_CbCr) + hor_final_Cr_index_block];
			pOutput_row_first_sample = final_output_buf_C;
			//pointer to first vertical accumulator
			pFirst_ver_accumulator = &ver_partsum_Cr[block_first_ver_accumulator_Cr];
			pFirst_ver_bilinear_sample = &ver_bilinear_Cr1[block_first_ver_accumulator_Cr];
			
			
			flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
			for(ver_counter=0;ver_counter<idct_row_num_CbCr;ver_counter++){ //vertical counter
				pCurrent_ver_accumulator = pFirst_ver_accumulator;
				flag_final_output_next_row = 0;
				block_hor_integer_counter = hor_integer_counter_Cr;
				temp_hor_partsum_count = hor_partsum_count_Cr; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Cr; //counter for horizontal fractional scaling

				if(1 == m_trans_ver_no_scale)
					pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
					
				for(hor_counter=0;hor_counter<idct_col_num_CbCr;hor_counter++){ //horizontal counter
					hor_partsum_Cr[ver_counter] += *pIDCT_Output; //sum for horizontal scaling

					if(1 == m_trans_hor_no_scale)
						hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter];
					
					pIDCT_Output++;
					temp_hor_partsum_count++; 
					if(temp_hor_partsum_count>=hor_scale_ratio_CbCr_int){ //horizontal ready to output one sample
						block_hor_integer_counter++;
						if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
							hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter];
						}
						else{ //ready to do horizontal linear interpolation
							fraction = temp_hor_fraction_counter & scale_bits_field;
							temp_output = hor_bilinear_Cr1[ver_counter]*(scale_fra_total_value-fraction) + hor_partsum_Cr[ver_counter]*fraction;
							//add to vertical accumulator
							*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling

							if(1 == m_trans_ver_no_scale)
								*pCurrent_ver_blinear_sample++ = *pCurrent_ver_accumulator;
							
							pCurrent_ver_accumulator++; //point to next vertical sum
							temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_CbCr_fraction;
							//update hor_bilinear_Cr1 since fractional scale factor >= 1.0
							hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter]; //bugfix001
						}
						hor_partsum_Cr[ver_counter] = 0; //clear horizontal sum since it has been added to vertical sum
						temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
					}
				}
				if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
					block_ver_partsum_count_Cr++; 
					vertical_para_change = 1;
					current_sample_pos = hor_final_Cr_index_block;
					if(block_ver_partsum_count_Cr >= ver_scale_ratio_CbCr_int){//vertical ready to output one sample
						hor_final_output_counter_Cr = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
						pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
						pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
						pOutput_current_sample = pOutput_row_first_sample;
						block_ver_integer_counter_Cr++;
						if(block_ver_integer_counter_Cr<=(block_ver_fraction_counter_Cr>>scale_precision)){//not ready to do interplation yet
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
						}
						else{//ready to do vertical linear interpolation
							fraction = block_ver_fraction_counter_Cr & scale_bits_field;
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
								temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
								if(0 == scale_factor_inverse_2D_Bits)
								{
									temp_final_output = temp_output / scale_factor_inverse_2D_CbCr;
								}
								else
								{
									temp_final_output = temp_output * scale_factor_inverse_2D_CbCr;
									temp_final_output >>= scale_factor_inverse_2D_Bits;
								}
							//	libc_printf("current_sample_pos:%d\n",current_sample_pos);
							//	libc_printf("block_ver_final_output_counter_Cr:%d\n",block_ver_final_output_counter_Cr);
							//	libc_printf("ver_final_Cr_index_block:%d\n",ver_final_Cr_index_block);
								x_mb_pos = (current_sample_pos>>3);
								x_pos = (current_sample_pos - ((current_sample_pos>>3)<<3));
								y_mb_pos = ((block_ver_final_output_counter_Cr+ver_final_Cr_index_block)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = (block_ver_final_output_counter_Cr+ver_final_Cr_index_block) - (((block_ver_final_output_counter_Cr+ver_final_Cr_index_block)>>4)<<4);
//
							//	libc_printf("x_pos:%d y_pos:%d\n",x_pos,y_pos);

																					//	libc_printf("pos2:%x\n", y_mb_pos*45*16*16 + (x_mb_pos<<8) + y_pos*16 + x_pos*2);
								pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + (x_pos<<1)] = (uchar)clamp(temp_final_output); 

	//							libc_printf("sample:%x\n",(uchar)clamp(temp_final_output));
							//	*pOutput_current_sample = (uchar)clamp(temp_final_output); 

	
								//update ver_bilinear_Cr1 since fractional scale factor >= 1.0
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//	pOutput_current_sample++;
								current_sample_pos++;
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
							block_ver_fraction_counter_Cr = block_ver_fraction_counter_Cr + ver_scale_ratio_CbCr_fraction;
							flag_final_output_next_row = 1;
						}
						block_ver_partsum_count_Cr = 0; //clear vertical partsum counter
					}	
				}
				if(flag_final_output_next_row!=0){ //indicating final output will move to next row
				//	pOutput_row_first_sample += Final_Output_Buf_Stride_CbCr; //final output move to next row
					block_ver_final_output_counter_Cr++;
				}
			}
			hor_partsum_count_Cr = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Cr = temp_hor_fraction_counter;
			hor_integer_counter_Cr = block_hor_integer_counter;
			if(vertical_para_change != 0){
				next_row_ver_partsum_count_Cr = block_ver_partsum_count_Cr;
				next_row_ver_integer_counter_Cr=block_ver_integer_counter_Cr;
				next_row_ver_fraction_counter_Cr = block_ver_fraction_counter_Cr;
				row_ver_final_output_counter_Cr = block_ver_final_output_counter_Cr; //counter for vertital final output
				block_first_ver_accumulator_Cr = block_first_ver_accumulator_Cr + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			}
			//if(block_ver_partsum_count_Cr != row_ver_partsum_count_Cr)
			//	next_row_ver_partsum_count_Cr = block_ver_partsum_count_Cr;
			//block_ver_final_output_counter_Cr and hor_final_output_counter_Cr will be use to update output address for next block
		}
	}
}

static void IDCT_BilinearScaling_90A(int block_type)
{
//This is a simplified version from EnhancedBilinearScaling()
//This function can only process 1.0 < scale_factor < 2.0
//So there is no integer scaling factor. So the program run faster than Enhanced Bilinear scaling
    //manson:  do scaling for this block
	int ver_counter;
	int hor_counter;
	int temp_hor_partsum_count = 0;
	int block_hor_integer_counter = 0;
	int temp_hor_fraction_counter = 0;
	int temp_final_output;
	int flag_final_output_next_row; //indicating final output point will move to next row
	int *pIDCT_Output; //bugfix001 use 32 bits for IDCT buffer

	uchar *pOutput_row_first_sample;
	uchar *pOutput_current_sample;

	uchar *pOutput_current_line;
	int *pFirst_ver_accumulator;   //32 bits
	int *pFirst_ver_bilinear_sample; //point to the first vertical bilinear input sample to be processed
	int *pCurrent_ver_accumulator = NULL; //used for vertical accumulating 
	int *pCurrent_ver_accumulator2; //use for vertical accumulator output
	int *pCurrent_ver_blinear_sample; //used for vertical bilinear interpolation 
	int temp_output; 
	int fraction;
	int temp_ver_index; //bugfix002
	int i;
	int debug_temp;
	//if the horizontal scaling factor is too large, for some MCU the veritcal parameters
	//below is not changed at all while for other MCUs it is change
	//it is necessary to record the vertical parameter change for the next row
	int vertical_para_change = 0; //indicating vertical parameter is changed
	int block_ver_partsum_count_Cb; //this one is used when processing a new block
	int block_ver_partsum_count_Cr; //this one is used when processing a new block
	int block_ver_integer_counter_Cb; //used in processing one block, and copy to MCU counter for next MCU row
	int block_ver_fraction_counter_Cb;
	int block_ver_integer_counter_Cr; //used in processing one block, and copy to MCU counter for next MCU row
	int block_ver_fraction_counter_Cr;
	int block_ver_final_output_counter_Cb; //counter for vertital final output
    	int block_ver_final_output_counter_Cr; //counter for vertital final output

	int block_hor_final_output_counter_Cb; //counter for vertital final output
    	int block_hor_final_output_counter_Cr; //counter for vertital final output
    	
	int block_curn_ver_accumulator_Y; //change016
	int block_curn_ver_accumulator_Y2; //change016

	int current_sample_pos=0;
	int x_pos,y_pos,x_mb_pos, y_mb_pos;
	
	//JPEG_ENTRY;
	//hor_final_output_counter and ver_final_output_counter will be used to update output address
	pIDCT_Output = temp_IDCT_buf2;

	if(block_type==0){ //current block is Y
		//manson: now we do transform for this block
		if(!g_imagedec_m33_combine_hw_sw_flag)
		{		
			switch(IDCT_mode_Y){
				case 0:
					idct(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 1:
					idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 2:
					idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 3:
					idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
			}
		}
		//change016
		//block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		//block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		//block_ver_partsum_count_Y = row_ver_partsum_count_Y; //change 018,vertical counters will be updated when preparing next row of MCU
		block_hor_final_output_counter_Y = 0;
		block_ver_final_output_counter_Y = 0;
		//pointer to first row's first sample 
		pOutput_row_first_sample = final_output_buf_Y;
		//pointer to first vertical accumulator
		if(MCU_hor_partsum_first==0){ //for 4:2:0 which has 4 Y blocks
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y1]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y1];
		}
		else{
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y2]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y2];
		}
		
		flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
		for(ver_counter=0;ver_counter<idct_row_num_Y;ver_counter++){ //vertical counter
			pCurrent_ver_accumulator = pFirst_ver_accumulator;
			flag_final_output_next_row = 0;
				
			if(MCU_hor_partsum_first==0){//bugfix002
				block_hor_integer_counter = hor_integer_counter_Y1;
				temp_hor_partsum_count = hor_partsum_count_Y1; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y1; //counter for horizontal fractional scaling
			}
			else{
				block_hor_integer_counter = hor_integer_counter_Y2;
				temp_hor_partsum_count = hor_partsum_count_Y2; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y2; //counter for horizontal fractional scaling
			}
			temp_ver_index = MCU_hor_partsum_first+ver_counter;//bugfix002
			for(hor_counter=0;hor_counter<idct_col_num_Y;hor_counter++){ //horizontal counter
				hor_partsum_Y[temp_ver_index] += *pIDCT_Output; //bugfix002//sum for horizontal scaling
				pIDCT_Output++;
				temp_hor_partsum_count++; 
				if(temp_hor_partsum_count>=hor_scale_ratio_Y_int){ //horizontal ready to output one sample
					block_hor_integer_counter++;
					if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
						hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];//bugfix002
					}
					else{ //ready to do horizontal linear interpolation
						fraction = temp_hor_fraction_counter & scale_bits_field;
						temp_output = hor_bilinear_Y1[temp_ver_index]*(scale_fra_total_value-fraction) + hor_partsum_Y[temp_ver_index]*fraction;//bugfix002
						//add to vertical accumulator
						*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
						pCurrent_ver_accumulator++; //point to next vertical sum
						temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_Y_fraction;
						//update hor_bilinear_Y1 since fractional scale factor >= 1.0
						hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];//bugfix002
					}
					hor_partsum_Y[temp_ver_index] = 0; //bugfix002//clear horizontal sum since it has been added to vertical sum
					temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
				}
			}
			pIDCT_Output += 8 - hor_counter;
			if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
				block_ver_partsum_count_Y++;
				vertical_para_change = 1;
#if 0				
				current_sample_pos = hor_final_Y_index_block;
#else
				current_sample_pos = ver_final_Y_index_block;
#endif
				if(block_ver_partsum_count_Y >= ver_scale_ratio_Y_int){//vertical ready to output one sample
#if 0				
					block_hor_final_output_counter_Y = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#else
					block_ver_final_output_counter_Y = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#endif
					pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
					pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
					pOutput_current_sample = pOutput_row_first_sample;
					block_ver_integer_counter_Y++;
					if(block_ver_integer_counter_Y<=(block_ver_fraction_counter_Y>>scale_precision)){//not ready to do interplation yet
						while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
							*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							pCurrent_ver_accumulator2++;
							pCurrent_ver_blinear_sample++;
						}
					}
					else{//ready to do vertical linear interpolation
						fraction = block_ver_fraction_counter_Y & scale_bits_field;
						while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
							temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
							temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
							temp_final_output = temp_output * scale_factor_inverse_2D_Y;
							temp_final_output >>= 15;
							//*pOutput_current_sample = (uchar)clamp(temp_final_output); 
#if 0						
							x_mb_pos = (current_sample_pos>>4);
							x_pos = (current_sample_pos - ((current_sample_pos>>4)<<4));
							y_mb_pos = ((block_ver_final_output_counter_Y+ver_final_Y_index_block)>>4);
							y_pos = (block_ver_final_output_counter_Y+ver_final_Y_index_block) -(((block_ver_final_output_counter_Y+ver_final_Y_index_block)>>4)<<4);
#else
							y_mb_pos = (current_sample_pos>>4);
							y_pos = (current_sample_pos - ((current_sample_pos>>4)<<4));
							x_mb_pos = ((hor_final_Y_index_block - block_hor_final_output_counter_Y)>>4);
							x_pos = (hor_final_Y_index_block - block_hor_final_output_counter_Y) -(((hor_final_Y_index_block - block_hor_final_output_counter_Y)>>4)<<4);							
#endif
							pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + x_pos] = (uchar)clamp(temp_final_output);
							//update ver_bilinear_Y1 since fractional scale factor >= 1.0
							*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
						//	pOutput_current_sample++;
#if 0						
							current_sample_pos++;
#else
							current_sample_pos++;
#endif
							pCurrent_ver_accumulator2++;
							pCurrent_ver_blinear_sample++;
						}
						block_ver_fraction_counter_Y = block_ver_fraction_counter_Y + ver_scale_ratio_Y_fraction;
						flag_final_output_next_row = 1;
					}
					block_ver_partsum_count_Y = 0; //clear vertical partsum counter
				}
			}
			if(flag_final_output_next_row!=0){ //indicating final output will move to next row
			//	pOutput_row_first_sample += Final_Output_Buf_Stride_Y; //final output move to next row
#if 0			
				block_ver_final_output_counter_Y++;
#else
				block_hor_final_output_counter_Y++;
#endif
			}
		}
		if(MCU_hor_partsum_first==0){//bugfix002
			hor_partsum_count_Y1 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y1 = temp_hor_fraction_counter;
			hor_integer_counter_Y1 = block_hor_integer_counter;
		}
		else{
			hor_partsum_count_Y2 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y2 = temp_hor_fraction_counter;
			hor_integer_counter_Y2 = block_hor_integer_counter;
		}
		if(vertical_para_change != 0){
			next_row_ver_partsum_count_Y = block_ver_partsum_count_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_integer_counter_Y = block_ver_integer_counter_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_fraction_counter_Y = block_ver_fraction_counter_Y; //for 4:2:0, the last Y block para is used
			if(MCU_hor_partsum_first==0)
				block_first_ver_accumulator_Y1 = block_first_ver_accumulator_Y1 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			else
				block_first_ver_accumulator_Y2 = block_first_ver_accumulator_Y2 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
		}
		//block_ver_final_output_counter_Y and block_hor_final_output_counter_Y will be use to update output address for next block
	}
	else{ //Cb or Cr
		//manson: now we do transform for this block
		if(!g_imagedec_m33_combine_hw_sw_flag)
		{			
			switch(IDCT_mode_CbCr){
				case 0:
					idct(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 1:
					idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 2:
					idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 3:
					idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 8:
					idct8x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 9:
					idct4x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 10:
					idct2x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 17:
					idct8x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 18:
					idct4x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
			}
		}
	//	libc_printf("IDCT_mode_CbCr:%d\n",IDCT_mode_CbCr);SDBBP();
		if(flag_CbCr_Hor_copy){
			for(i=127;i>=0;i--){
				temp_IDCT_buf2[i] = temp_IDCT_buf2[i>>1];
			}
		}
		if(block_type==1){ //current block is Cb
			//manson: because CbCr may have larger scale factor, 
			//so CbCr still use enhanced bilinear scaling
			block_ver_integer_counter_Cb = row_ver_integer_counter_Cb; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Cb = row_ver_fraction_counter_Cb;
			block_ver_partsum_count_Cb = row_ver_partsum_count_Cb;		
			hor_final_output_counter_Cb = 0;
			block_ver_final_output_counter_Cb = 0;
#if 1			
			ver_final_output_counter_Cb = 0;
			block_hor_final_output_counter_Cb = 0;
#endif
			//pointer to first row's first sample 
			//pOutput_row_first_sample = &final_output_buf_Cb[(ver_final_Cb_index_block*Final_Output_Buf_Stride_CbCr) + hor_final_Cb_index_block];
			pOutput_row_first_sample = final_output_buf_C;
			//pointer to first vertical accumulator
			//manson: if scale factor is too large, some MCU rows may not have final output
			//so we can not take the final output index as the vertical accumulator index (although for small scale factor, they are equivalent)
			pFirst_ver_accumulator = &ver_partsum_Cb[block_first_ver_accumulator_Cb];
			pFirst_ver_bilinear_sample = &ver_bilinear_Cb1[block_first_ver_accumulator_Cb];
			flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
			for(ver_counter=0;ver_counter<idct_row_num_CbCr;ver_counter++){ //vertical counter
				pCurrent_ver_accumulator = pFirst_ver_accumulator;
				flag_final_output_next_row = 0;
				block_hor_integer_counter = hor_integer_counter_Cb;
				temp_hor_partsum_count = hor_partsum_count_Cb; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Cb; //counter for horizontal fractional scaling
		
				for(hor_counter=0;hor_counter<idct_col_num_CbCr;hor_counter++){ //horizontal counter
					hor_partsum_Cb[ver_counter] += *pIDCT_Output; //sum for horizontal scaling
					pIDCT_Output++;
					temp_hor_partsum_count++; 
					if(temp_hor_partsum_count>=hor_scale_ratio_CbCr_int){ //horizontal ready to output one sample
						block_hor_integer_counter++;
						if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
							hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];
						}
						else{ //ready to do horizontal linear interpolation
							fraction = temp_hor_fraction_counter & scale_bits_field;
							temp_output = hor_bilinear_Cb1[ver_counter]*(scale_fra_total_value-fraction) + hor_partsum_Cb[ver_counter]*fraction;
							//add to vertical accumulator
							*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
							pCurrent_ver_accumulator++; //point to next vertical sum
							temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_CbCr_fraction;
							//update hor_bilinear_Cb1 since fractional scale factor >= 1.0
							hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];
						}
						hor_partsum_Cb[ver_counter] = 0; //clear horizontal sum since it has been added to vertical sum
						temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
					}
				}
				if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
					block_ver_partsum_count_Cb++; 
					vertical_para_change = 1;
#if 0					
					current_sample_pos = hor_final_Cb_index_block;
#else
					current_sample_pos = ver_final_Cb_index_block;
#endif
					if(block_ver_partsum_count_Cb >= ver_scale_ratio_CbCr_int){//vertical ready to output one sample
#if 0					
						hor_final_output_counter_Cb = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#else
						ver_final_output_counter_Cb = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#endif
						pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
						pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
						pOutput_current_sample = pOutput_row_first_sample;
						block_ver_integer_counter_Cb++;
						if(block_ver_integer_counter_Cb<=(block_ver_fraction_counter_Cb>>scale_precision)){//not ready to do interplation yet
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
						}	
						else{//ready to do vertical linear interpolation
							fraction = block_ver_fraction_counter_Cb & scale_bits_field;
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
								temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
								temp_final_output = temp_output * scale_factor_inverse_2D_CbCr;
								temp_final_output >>= 15;

#if 0
								x_mb_pos = (current_sample_pos>>3);
								x_pos = (current_sample_pos - ((current_sample_pos>>3)<<3));
								y_mb_pos = ((block_ver_final_output_counter_Cb+ver_final_Cb_index_block)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = (block_ver_final_output_counter_Cb+ver_final_Cb_index_block) - (((block_ver_final_output_counter_Cb+ver_final_Cb_index_block)>>4)<<4);
#else
								x_mb_pos = ((hor_final_Cb_index_block - block_hor_final_output_counter_Cb)>>3);
								x_pos = ((hor_final_Cb_index_block - block_hor_final_output_counter_Cb) - (((hor_final_Cb_index_block - block_hor_final_output_counter_Cb)>>3)<<3));
								y_mb_pos = ((current_sample_pos)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = current_sample_pos - ((current_sample_pos>>4)<<4);
#endif	
							
							//	libc_printf("x_:%d y_:%d\n",current_sample_pos,(block_ver_final_output_counter_Cb+ver_final_Cb_index_block));
							//	libc_printf("pos1:%x\n", y_mb_pos*45*16*16 + (x_mb_pos<<8) + y_pos*16 + x_pos*2 + 1);
								pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8)+ (x_mb_pos<<8) + (y_pos<<4) + (x_pos<<1) + 1] = (uchar)clamp(temp_final_output); 

								//update ver_bilinear_Cb1 since fractional scale factor >= 1.0
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//	pOutput_current_sample++;
#if 0							
								current_sample_pos++;
#else
								current_sample_pos++;
#endif
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
							block_ver_fraction_counter_Cb = block_ver_fraction_counter_Cb + ver_scale_ratio_CbCr_fraction;
							flag_final_output_next_row = 1;
						}
						block_ver_partsum_count_Cb = 0; //clear vertical partsum counter
					}
				}
				if(flag_final_output_next_row!=0){ //indicating final output will move to next row
				//	pOutput_row_first_sample += Final_Output_Buf_Stride_CbCr; //final output move to next row
#if 0				
					block_ver_final_output_counter_Cb++;
#else
					block_hor_final_output_counter_Cb++;
#endif
				}
			}
			hor_partsum_count_Cb = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Cb = temp_hor_fraction_counter;
			hor_integer_counter_Cb = block_hor_integer_counter;
			if(vertical_para_change != 0){
				next_row_ver_partsum_count_Cb = block_ver_partsum_count_Cb;
				next_row_ver_integer_counter_Cb=block_ver_integer_counter_Cb;
				next_row_ver_fraction_counter_Cb = block_ver_fraction_counter_Cb;
#if 0				
				row_ver_final_output_counter_Cb = block_ver_final_output_counter_Cb; //counter for vertital final output
#else
				row_hor_final_output_counter_Cb = block_hor_final_output_counter_Cb; 	
#endif
				block_first_ver_accumulator_Cb = block_first_ver_accumulator_Cb + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			}
			//if(block_ver_partsum_count_Cb != row_ver_partsum_count_Cb)
			//	next_row_ver_partsum_count_Cb = block_ver_partsum_count_Cb;
			//block_ver_final_output_counter_Cb and hor_final_output_counter_Cb will be use to update output address for next block
		}
		else{//current block is Cr
			//for debug
			block_ver_integer_counter_Cr = row_ver_integer_counter_Cr;//vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Cr = row_ver_fraction_counter_Cr;
			block_ver_partsum_count_Cr = row_ver_partsum_count_Cr;
			hor_final_output_counter_Cr = 0;
			block_ver_final_output_counter_Cr = 0;
#if 1			
			ver_final_output_counter_Cr = 0;
			block_hor_final_output_counter_Cr = 0;
#endif		
			//pointer to first row's first sample 
		//	pOutput_row_first_sample = &final_output_buf_Cr[(ver_final_Cr_index_block*Final_Output_Buf_Stride_CbCr) + hor_final_Cr_index_block];
			pOutput_row_first_sample = final_output_buf_C;
			//pointer to first vertical accumulator
			pFirst_ver_accumulator = &ver_partsum_Cr[block_first_ver_accumulator_Cr];
			pFirst_ver_bilinear_sample = &ver_bilinear_Cr1[block_first_ver_accumulator_Cr];
			
			flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
			for(ver_counter=0;ver_counter<idct_row_num_CbCr;ver_counter++){ //vertical counter
				pCurrent_ver_accumulator = pFirst_ver_accumulator;
				flag_final_output_next_row = 0;
				block_hor_integer_counter = hor_integer_counter_Cr;
				temp_hor_partsum_count = hor_partsum_count_Cr; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Cr; //counter for horizontal fractional scaling
		
				for(hor_counter=0;hor_counter<idct_col_num_CbCr;hor_counter++){ //horizontal counter
					hor_partsum_Cr[ver_counter] += *pIDCT_Output; //sum for horizontal scaling
					pIDCT_Output++;
					temp_hor_partsum_count++; 
					if(temp_hor_partsum_count>=hor_scale_ratio_CbCr_int){ //horizontal ready to output one sample
						block_hor_integer_counter++;
						if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
							hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter];
						}
						else{ //ready to do horizontal linear interpolation
							fraction = temp_hor_fraction_counter & scale_bits_field;
							temp_output = hor_bilinear_Cr1[ver_counter]*(scale_fra_total_value-fraction) + hor_partsum_Cr[ver_counter]*fraction;
							//add to vertical accumulator
							*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
							pCurrent_ver_accumulator++; //point to next vertical sum
							temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_CbCr_fraction;
							//update hor_bilinear_Cr1 since fractional scale factor >= 1.0
							hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter]; //bugfix001
						}
						hor_partsum_Cr[ver_counter] = 0; //clear horizontal sum since it has been added to vertical sum
						temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
					}
				}
				if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
					block_ver_partsum_count_Cr++; 
					vertical_para_change = 1;
#if 0					
					current_sample_pos = hor_final_Cr_index_block;
#else
					current_sample_pos = ver_final_Cr_index_block;
#endif
					if(block_ver_partsum_count_Cr >= ver_scale_ratio_CbCr_int){//vertical ready to output one sample
#if 0					
						hor_final_output_counter_Cr = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#else
						ver_final_output_counter_Cr = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#endif
						pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
						pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
						pOutput_current_sample = pOutput_row_first_sample;
						block_ver_integer_counter_Cr++;
						if(block_ver_integer_counter_Cr<=(block_ver_fraction_counter_Cr>>scale_precision)){//not ready to do interplation yet
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
						}
						else{//ready to do vertical linear interpolation
							fraction = block_ver_fraction_counter_Cr & scale_bits_field;
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
								temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
								temp_final_output = temp_output * scale_factor_inverse_2D_CbCr;
								temp_final_output >>= 15;
							//	libc_printf("current_sample_pos:%d\n",current_sample_pos);
							//	libc_printf("block_ver_final_output_counter_Cr:%d\n",block_ver_final_output_counter_Cr);
							//	libc_printf("ver_final_Cr_index_block:%d\n",ver_final_Cr_index_block);
#if 0							
								x_mb_pos = (current_sample_pos>>3);
								x_pos = (current_sample_pos - ((current_sample_pos>>3)<<3));
								y_mb_pos = ((block_ver_final_output_counter_Cr+ver_final_Cr_index_block)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = (block_ver_final_output_counter_Cr+ver_final_Cr_index_block) - (((block_ver_final_output_counter_Cr+ver_final_Cr_index_block)>>4)<<4);
#else
								x_mb_pos = ((hor_final_Cr_index_block - block_hor_final_output_counter_Cr)>>3);
								x_pos = ((hor_final_Cr_index_block - block_hor_final_output_counter_Cr) - (((hor_final_Cr_index_block - block_hor_final_output_counter_Cr)>>3)<<3));
								y_mb_pos = ((current_sample_pos)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = current_sample_pos - ((current_sample_pos>>4)<<4);
#endif
//
							//	libc_printf("x_pos:%d y_pos:%d\n",x_pos,y_pos);

																					//	libc_printf("pos2:%x\n", y_mb_pos*45*16*16 + (x_mb_pos<<8) + y_pos*16 + x_pos*2);
								pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + (x_pos<<1)] = (uchar)clamp(temp_final_output); 

	//							libc_printf("sample:%x\n",(uchar)clamp(temp_final_output));
							//	*pOutput_current_sample = (uchar)clamp(temp_final_output); 

	
								//update ver_bilinear_Cr1 since fractional scale factor >= 1.0
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//	pOutput_current_sample++;
#if 0							
								current_sample_pos++;
#else
								current_sample_pos++;
#endif
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
							block_ver_fraction_counter_Cr = block_ver_fraction_counter_Cr + ver_scale_ratio_CbCr_fraction;
							flag_final_output_next_row = 1;
						}
						block_ver_partsum_count_Cr = 0; //clear vertical partsum counter
					}	
				}
				if(flag_final_output_next_row!=0){ //indicating final output will move to next row
				//	pOutput_row_first_sample += Final_Output_Buf_Stride_CbCr; //final output move to next row
#if 0				
					block_ver_final_output_counter_Cr++;
#else
					block_hor_final_output_counter_Cr++;
#endif
				}
			}
			hor_partsum_count_Cr = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Cr = temp_hor_fraction_counter;
			hor_integer_counter_Cr = block_hor_integer_counter;
			if(vertical_para_change != 0){
				next_row_ver_partsum_count_Cr = block_ver_partsum_count_Cr;
				next_row_ver_integer_counter_Cr=block_ver_integer_counter_Cr;
				next_row_ver_fraction_counter_Cr = block_ver_fraction_counter_Cr;
#if 0				
				row_ver_final_output_counter_Cr = block_ver_final_output_counter_Cr; //counter for vertital final output
#else
				row_hor_final_output_counter_Cr = block_hor_final_output_counter_Cr; //counter for vertital final output
#endif
				block_first_ver_accumulator_Cr = block_first_ver_accumulator_Cr + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			}
			//if(block_ver_partsum_count_Cr != row_ver_partsum_count_Cr)
			//	next_row_ver_partsum_count_Cr = block_ver_partsum_count_Cr;
			//block_ver_final_output_counter_Cr and hor_final_output_counter_Cr will be use to update output address for next block
		}
	}
	//JPEG_EXIT;
}
static void IDCT_BilinearScaling_180(int block_type)
{
//This is a simplified version from EnhancedBilinearScaling()
//This function can only process 1.0 < scale_factor < 2.0
//So there is no integer scaling factor. So the program run faster than Enhanced Bilinear scaling
    //manson:  do scaling for this block
	int ver_counter;
	int hor_counter;
	int temp_hor_partsum_count = 0;
	int block_hor_integer_counter = 0;
	int temp_hor_fraction_counter = 0;
	int temp_final_output;
	int flag_final_output_next_row; //indicating final output point will move to next row
	int *pIDCT_Output; //bugfix001 use 32 bits for IDCT buffer

	uchar *pOutput_row_first_sample;
	uchar *pOutput_current_sample;

	uchar *pOutput_current_line;
	int *pFirst_ver_accumulator;   //32 bits
	int *pFirst_ver_bilinear_sample; //point to the first vertical bilinear input sample to be processed
	int *pCurrent_ver_accumulator = NULL; //used for vertical accumulating 
	int *pCurrent_ver_accumulator2; //use for vertical accumulator output
	int *pCurrent_ver_blinear_sample; //used for vertical bilinear interpolation 
	int temp_output; 
	int fraction;
	int temp_ver_index; //bugfix002
	int i;
	int debug_temp;
	//if the horizontal scaling factor is too large, for some MCU the veritcal parameters
	//below is not changed at all while for other MCUs it is change
	//it is necessary to record the vertical parameter change for the next row
	int vertical_para_change = 0; //indicating vertical parameter is changed
	int block_ver_partsum_count_Cb; //this one is used when processing a new block
	int block_ver_partsum_count_Cr; //this one is used when processing a new block
	int block_ver_integer_counter_Cb; //used in processing one block, and copy to MCU counter for next MCU row
	int block_ver_fraction_counter_Cb;
	int block_ver_integer_counter_Cr; //used in processing one block, and copy to MCU counter for next MCU row
	int block_ver_fraction_counter_Cr;
	int block_ver_final_output_counter_Cb; //counter for vertital final output
    	int block_ver_final_output_counter_Cr; //counter for vertital final output

	int block_curn_ver_accumulator_Y; //change016
	int block_curn_ver_accumulator_Y2; //change016

	int current_sample_pos=0;
	int x_pos,y_pos,x_mb_pos, y_mb_pos;
	

	//hor_final_output_counter and ver_final_output_counter will be used to update output address
	pIDCT_Output = temp_IDCT_buf2;

	if(block_type==0){ //current block is Y
		//manson: now we do transform for this block
		if(!g_imagedec_m33_combine_hw_sw_flag)
		{		
			switch(IDCT_mode_Y){
				case 0:
					idct(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 1:
					idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 2:
					idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 3:
					idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
			}
		}
		//change016
		//block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		//block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		//block_ver_partsum_count_Y = row_ver_partsum_count_Y; //change 018,vertical counters will be updated when preparing next row of MCU
		block_hor_final_output_counter_Y = 0;
		block_ver_final_output_counter_Y = 0;
		//pointer to first row's first sample 
		pOutput_row_first_sample = final_output_buf_Y;
		//pointer to first vertical accumulator
		if(MCU_hor_partsum_first==0){ //for 4:2:0 which has 4 Y blocks
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y1]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y1];
		}
		else{
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y2]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y2];
		}
		
		flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
		for(ver_counter=0;ver_counter<idct_row_num_Y;ver_counter++){ //vertical counter
			pCurrent_ver_accumulator = pFirst_ver_accumulator;
			flag_final_output_next_row = 0;
				
			if(MCU_hor_partsum_first==0){//bugfix002
				block_hor_integer_counter = hor_integer_counter_Y1;
				temp_hor_partsum_count = hor_partsum_count_Y1; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y1; //counter for horizontal fractional scaling
			}
			else{
				block_hor_integer_counter = hor_integer_counter_Y2;
				temp_hor_partsum_count = hor_partsum_count_Y2; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y2; //counter for horizontal fractional scaling
			}
			temp_ver_index = MCU_hor_partsum_first+ver_counter;//bugfix002
			for(hor_counter=0;hor_counter<idct_col_num_Y;hor_counter++){ //horizontal counter
				hor_partsum_Y[temp_ver_index] += *pIDCT_Output; //bugfix002//sum for horizontal scaling
				pIDCT_Output++;
				temp_hor_partsum_count++; 
				if(temp_hor_partsum_count>=hor_scale_ratio_Y_int){ //horizontal ready to output one sample
					block_hor_integer_counter++;
					if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
						hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];//bugfix002
					}
					else{ //ready to do horizontal linear interpolation
						fraction = temp_hor_fraction_counter & scale_bits_field;
						temp_output = hor_bilinear_Y1[temp_ver_index]*(scale_fra_total_value-fraction) + hor_partsum_Y[temp_ver_index]*fraction;//bugfix002
						//add to vertical accumulator
						*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
						pCurrent_ver_accumulator++; //point to next vertical sum
						temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_Y_fraction;
						//update hor_bilinear_Y1 since fractional scale factor >= 1.0
						hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];//bugfix002
					}
					hor_partsum_Y[temp_ver_index] = 0; //bugfix002//clear horizontal sum since it has been added to vertical sum
					temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
				}
			}
			pIDCT_Output += 8 - hor_counter;
			if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
				block_ver_partsum_count_Y++;
				vertical_para_change = 1;
				current_sample_pos = hor_final_Y_index_block;
				if(block_ver_partsum_count_Y >= ver_scale_ratio_Y_int){//vertical ready to output one sample
					block_hor_final_output_counter_Y = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
					pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
					pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
					pOutput_current_sample = pOutput_row_first_sample;
					block_ver_integer_counter_Y++;
					if(block_ver_integer_counter_Y<=(block_ver_fraction_counter_Y>>scale_precision)){//not ready to do interplation yet
						while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
							*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							pCurrent_ver_accumulator2++;
							pCurrent_ver_blinear_sample++;
						}
					}
					else{//ready to do vertical linear interpolation
						fraction = block_ver_fraction_counter_Y & scale_bits_field;
						while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
							temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
							temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
							temp_final_output = temp_output * scale_factor_inverse_2D_Y;
							temp_final_output >>= 15;
							//*pOutput_current_sample = (uchar)clamp(temp_final_output); 
							x_mb_pos = (current_sample_pos>>4);
							x_pos = (current_sample_pos - ((current_sample_pos>>4)<<4));
							y_mb_pos = ((ver_final_Y_index_block - block_ver_final_output_counter_Y)>>4);
							y_pos = (ver_final_Y_index_block - block_ver_final_output_counter_Y) -(((ver_final_Y_index_block - block_ver_final_output_counter_Y)>>4)<<4);
							pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + x_pos] = (uchar)clamp(temp_final_output);	  
							//update ver_bilinear_Y1 since fractional scale factor >= 1.0
							*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
						//	pOutput_current_sample++;
							current_sample_pos--;
							pCurrent_ver_accumulator2++;
							pCurrent_ver_blinear_sample++;
						}
						block_ver_fraction_counter_Y = block_ver_fraction_counter_Y + ver_scale_ratio_Y_fraction;
						flag_final_output_next_row = 1;
					}
					block_ver_partsum_count_Y = 0; //clear vertical partsum counter
				}
			}
			if(flag_final_output_next_row!=0){ //indicating final output will move to next row
			//	pOutput_row_first_sample += Final_Output_Buf_Stride_Y; //final output move to next row
				block_ver_final_output_counter_Y++;
			}
		}
		if(MCU_hor_partsum_first==0){//bugfix002
			hor_partsum_count_Y1 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y1 = temp_hor_fraction_counter;
			hor_integer_counter_Y1 = block_hor_integer_counter;
		}
		else{
			hor_partsum_count_Y2 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y2 = temp_hor_fraction_counter;
			hor_integer_counter_Y2 = block_hor_integer_counter;
		}
		if(vertical_para_change != 0){
			next_row_ver_partsum_count_Y = block_ver_partsum_count_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_integer_counter_Y = block_ver_integer_counter_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_fraction_counter_Y = block_ver_fraction_counter_Y; //for 4:2:0, the last Y block para is used
			if(MCU_hor_partsum_first==0)
				block_first_ver_accumulator_Y1 = block_first_ver_accumulator_Y1 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			else
				block_first_ver_accumulator_Y2 = block_first_ver_accumulator_Y2 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
		}
		//block_ver_final_output_counter_Y and block_hor_final_output_counter_Y will be use to update output address for next block
	}
	else{ //Cb or Cr
		//manson: now we do transform for this block
		if(!g_imagedec_m33_combine_hw_sw_flag)
		{		
			switch(IDCT_mode_CbCr){
				case 0:
					idct(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 1:
					idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 2:
					idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 3:
					idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 8:
					idct8x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 9:
					idct4x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 10:
					idct2x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 17:
					idct8x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 18:
					idct4x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
			}
		}
	//	libc_printf("IDCT_mode_CbCr:%d\n",IDCT_mode_CbCr);SDBBP();
		if(flag_CbCr_Hor_copy){
			for(i=127;i>=0;i--){
				temp_IDCT_buf2[i] = temp_IDCT_buf2[i>>1];
			}
		}
		if(block_type==1){ //current block is Cb
			//manson: because CbCr may have larger scale factor, 
			//so CbCr still use enhanced bilinear scaling
			block_ver_integer_counter_Cb = row_ver_integer_counter_Cb; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Cb = row_ver_fraction_counter_Cb;
			block_ver_partsum_count_Cb = row_ver_partsum_count_Cb;
			hor_final_output_counter_Cb = 0;
			block_ver_final_output_counter_Cb = 0;
			//pointer to first row's first sample 
			//pOutput_row_first_sample = &final_output_buf_Cb[(ver_final_Cb_index_block*Final_Output_Buf_Stride_CbCr) + hor_final_Cb_index_block];
			pOutput_row_first_sample = final_output_buf_C;
			//pointer to first vertical accumulator
			//manson: if scale factor is too large, some MCU rows may not have final output
			//so we can not take the final output index as the vertical accumulator index (although for small scale factor, they are equivalent)
			pFirst_ver_accumulator = &ver_partsum_Cb[block_first_ver_accumulator_Cb];
			pFirst_ver_bilinear_sample = &ver_bilinear_Cb1[block_first_ver_accumulator_Cb];
			flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
			for(ver_counter=0;ver_counter<idct_row_num_CbCr;ver_counter++){ //vertical counter
				pCurrent_ver_accumulator = pFirst_ver_accumulator;
				flag_final_output_next_row = 0;
				block_hor_integer_counter = hor_integer_counter_Cb;
				temp_hor_partsum_count = hor_partsum_count_Cb; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Cb; //counter for horizontal fractional scaling
		
				for(hor_counter=0;hor_counter<idct_col_num_CbCr;hor_counter++){ //horizontal counter
					hor_partsum_Cb[ver_counter] += *pIDCT_Output; //sum for horizontal scaling
					pIDCT_Output++;
					temp_hor_partsum_count++; 
					if(temp_hor_partsum_count>=hor_scale_ratio_CbCr_int){ //horizontal ready to output one sample
						block_hor_integer_counter++;
						if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
							hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];
						}
						else{ //ready to do horizontal linear interpolation
							fraction = temp_hor_fraction_counter & scale_bits_field;
							temp_output = hor_bilinear_Cb1[ver_counter]*(scale_fra_total_value-fraction) + hor_partsum_Cb[ver_counter]*fraction;
							//add to vertical accumulator
							*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
							pCurrent_ver_accumulator++; //point to next vertical sum
							temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_CbCr_fraction;
							//update hor_bilinear_Cb1 since fractional scale factor >= 1.0
							hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];
						}
						hor_partsum_Cb[ver_counter] = 0; //clear horizontal sum since it has been added to vertical sum
						temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
					}
				}
				if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
					block_ver_partsum_count_Cb++; 
					vertical_para_change = 1;
					current_sample_pos = hor_final_Cb_index_block;
					if(block_ver_partsum_count_Cb >= ver_scale_ratio_CbCr_int){//vertical ready to output one sample
						hor_final_output_counter_Cb = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
						pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
						pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
						pOutput_current_sample = pOutput_row_first_sample;
						block_ver_integer_counter_Cb++;
						if(block_ver_integer_counter_Cb<=(block_ver_fraction_counter_Cb>>scale_precision)){//not ready to do interplation yet
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
						}	
						else{//ready to do vertical linear interpolation
							fraction = block_ver_fraction_counter_Cb & scale_bits_field;
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
								temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
								temp_final_output = temp_output * scale_factor_inverse_2D_CbCr;
								temp_final_output >>= 15;

								x_mb_pos = (current_sample_pos>>3);
								x_pos = (current_sample_pos - ((current_sample_pos>>3)<<3));
								y_mb_pos = ((ver_final_Cb_index_block - block_ver_final_output_counter_Cb)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = (ver_final_Cb_index_block - block_ver_final_output_counter_Cb) - (((ver_final_Cb_index_block - block_ver_final_output_counter_Cb)>>4)<<4);
							//	libc_printf("x_:%d y_:%d\n",current_sample_pos,(block_ver_final_output_counter_Cb+ver_final_Cb_index_block));
							//	libc_printf("pos1:%x\n", y_mb_pos*45*16*16 + (x_mb_pos<<8) + y_pos*16 + x_pos*2 + 1);
								pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8)+ (x_mb_pos<<8) + (y_pos<<4) + (x_pos<<1) + 1] = (uchar)clamp(temp_final_output); 


								//update ver_bilinear_Cb1 since fractional scale factor >= 1.0
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//	pOutput_current_sample++;
								current_sample_pos--;
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
							block_ver_fraction_counter_Cb = block_ver_fraction_counter_Cb + ver_scale_ratio_CbCr_fraction;
							flag_final_output_next_row = 1;
						}
						block_ver_partsum_count_Cb = 0; //clear vertical partsum counter
					}
				}
				if(flag_final_output_next_row!=0){ //indicating final output will move to next row
				//	pOutput_row_first_sample += Final_Output_Buf_Stride_CbCr; //final output move to next row
					block_ver_final_output_counter_Cb++;
				}
			}
			hor_partsum_count_Cb = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Cb = temp_hor_fraction_counter;
			hor_integer_counter_Cb = block_hor_integer_counter;
			if(vertical_para_change != 0){
				next_row_ver_partsum_count_Cb = block_ver_partsum_count_Cb;
				next_row_ver_integer_counter_Cb=block_ver_integer_counter_Cb;
				next_row_ver_fraction_counter_Cb = block_ver_fraction_counter_Cb;
				row_ver_final_output_counter_Cb = block_ver_final_output_counter_Cb; //counter for vertital final output
				block_first_ver_accumulator_Cb = block_first_ver_accumulator_Cb + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			}
			//if(block_ver_partsum_count_Cb != row_ver_partsum_count_Cb)
			//	next_row_ver_partsum_count_Cb = block_ver_partsum_count_Cb;
			//block_ver_final_output_counter_Cb and hor_final_output_counter_Cb will be use to update output address for next block
		}
		else{//current block is Cr
			//for debug
			block_ver_integer_counter_Cr = row_ver_integer_counter_Cr;//vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Cr = row_ver_fraction_counter_Cr;
			block_ver_partsum_count_Cr = row_ver_partsum_count_Cr;
			hor_final_output_counter_Cr = 0;
			block_ver_final_output_counter_Cr = 0;
			//pointer to first row's first sample 
		//	pOutput_row_first_sample = &final_output_buf_Cr[(ver_final_Cr_index_block*Final_Output_Buf_Stride_CbCr) + hor_final_Cr_index_block];
			pOutput_row_first_sample = final_output_buf_C;
			//pointer to first vertical accumulator
			pFirst_ver_accumulator = &ver_partsum_Cr[block_first_ver_accumulator_Cr];
			pFirst_ver_bilinear_sample = &ver_bilinear_Cr1[block_first_ver_accumulator_Cr];
			
			
			flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
			for(ver_counter=0;ver_counter<idct_row_num_CbCr;ver_counter++){ //vertical counter
				pCurrent_ver_accumulator = pFirst_ver_accumulator;
				flag_final_output_next_row = 0;
				block_hor_integer_counter = hor_integer_counter_Cr;
				temp_hor_partsum_count = hor_partsum_count_Cr; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Cr; //counter for horizontal fractional scaling
		
				for(hor_counter=0;hor_counter<idct_col_num_CbCr;hor_counter++){ //horizontal counter
					hor_partsum_Cr[ver_counter] += *pIDCT_Output; //sum for horizontal scaling
					pIDCT_Output++;
					temp_hor_partsum_count++; 
					if(temp_hor_partsum_count>=hor_scale_ratio_CbCr_int){ //horizontal ready to output one sample
						block_hor_integer_counter++;
						if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
							hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter];
						}
						else{ //ready to do horizontal linear interpolation
							fraction = temp_hor_fraction_counter & scale_bits_field;
							temp_output = hor_bilinear_Cr1[ver_counter]*(scale_fra_total_value-fraction) + hor_partsum_Cr[ver_counter]*fraction;
							//add to vertical accumulator
							*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
							pCurrent_ver_accumulator++; //point to next vertical sum
							temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_CbCr_fraction;
							//update hor_bilinear_Cr1 since fractional scale factor >= 1.0
							hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter]; //bugfix001
						}
						hor_partsum_Cr[ver_counter] = 0; //clear horizontal sum since it has been added to vertical sum
						temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
					}
				}
				if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
					block_ver_partsum_count_Cr++; 
					vertical_para_change = 1;
					current_sample_pos = hor_final_Cr_index_block;
					if(block_ver_partsum_count_Cr >= ver_scale_ratio_CbCr_int){//vertical ready to output one sample
						hor_final_output_counter_Cr = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
						pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
						pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
						pOutput_current_sample = pOutput_row_first_sample;
						block_ver_integer_counter_Cr++;
						if(block_ver_integer_counter_Cr<=(block_ver_fraction_counter_Cr>>scale_precision)){//not ready to do interplation yet
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
						}
						else{//ready to do vertical linear interpolation
							fraction = block_ver_fraction_counter_Cr & scale_bits_field;
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
								temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
								temp_final_output = temp_output * scale_factor_inverse_2D_CbCr;
								temp_final_output >>= 15;
							//	libc_printf("current_sample_pos:%d\n",current_sample_pos);
							//	libc_printf("block_ver_final_output_counter_Cr:%d\n",block_ver_final_output_counter_Cr);
							//	libc_printf("ver_final_Cr_index_block:%d\n",ver_final_Cr_index_block);
								x_mb_pos = (current_sample_pos>>3);
								x_pos = (current_sample_pos - ((current_sample_pos>>3)<<3));
								y_mb_pos = ((ver_final_Cr_index_block - block_ver_final_output_counter_Cr)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = (ver_final_Cr_index_block - block_ver_final_output_counter_Cr) - (((ver_final_Cr_index_block - block_ver_final_output_counter_Cr)>>4)<<4);
//
							//	libc_printf("x_pos:%d y_pos:%d\n",x_pos,y_pos);

																					//	libc_printf("pos2:%x\n", y_mb_pos*45*16*16 + (x_mb_pos<<8) + y_pos*16 + x_pos*2);
								pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + (x_pos<<1)] = (uchar)clamp(temp_final_output); 

	//							libc_printf("sample:%x\n",(uchar)clamp(temp_final_output));
							//	*pOutput_current_sample = (uchar)clamp(temp_final_output); 

	
								//update ver_bilinear_Cr1 since fractional scale factor >= 1.0
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//	pOutput_current_sample++;
								current_sample_pos--;
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
							block_ver_fraction_counter_Cr = block_ver_fraction_counter_Cr + ver_scale_ratio_CbCr_fraction;
							flag_final_output_next_row = 1;
						}
						block_ver_partsum_count_Cr = 0; //clear vertical partsum counter
					}	
				}
				if(flag_final_output_next_row!=0){ //indicating final output will move to next row
				//	pOutput_row_first_sample += Final_Output_Buf_Stride_CbCr; //final output move to next row
					block_ver_final_output_counter_Cr++;
				}
			}
			hor_partsum_count_Cr = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Cr = temp_hor_fraction_counter;
			hor_integer_counter_Cr = block_hor_integer_counter;
			if(vertical_para_change != 0){
				next_row_ver_partsum_count_Cr = block_ver_partsum_count_Cr;
				next_row_ver_integer_counter_Cr=block_ver_integer_counter_Cr;
				next_row_ver_fraction_counter_Cr = block_ver_fraction_counter_Cr;
				row_ver_final_output_counter_Cr = block_ver_final_output_counter_Cr; //counter for vertital final output
				block_first_ver_accumulator_Cr = block_first_ver_accumulator_Cr + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			}
			//if(block_ver_partsum_count_Cr != row_ver_partsum_count_Cr)
			//	next_row_ver_partsum_count_Cr = block_ver_partsum_count_Cr;
			//block_ver_final_output_counter_Cr and hor_final_output_counter_Cr will be use to update output address for next block
		}
	}
}

static void IDCT_BilinearScaling_90C(int block_type)
{
//This is a simplified version from EnhancedBilinearScaling()
//This function can only process 1.0 < scale_factor < 2.0
//So there is no integer scaling factor. So the program run faster than Enhanced Bilinear scaling
    //manson:  do scaling for this block
	int ver_counter;
	int hor_counter;
	int temp_hor_partsum_count = 0;
	int block_hor_integer_counter = 0;
	int temp_hor_fraction_counter = 0;
	int temp_final_output;
	int flag_final_output_next_row; //indicating final output point will move to next row
	int *pIDCT_Output; //bugfix001 use 32 bits for IDCT buffer

	uchar *pOutput_row_first_sample;
	uchar *pOutput_current_sample;

	uchar *pOutput_current_line;
	int *pFirst_ver_accumulator;   //32 bits
	int *pFirst_ver_bilinear_sample; //point to the first vertical bilinear input sample to be processed
	int *pCurrent_ver_accumulator = NULL; //used for vertical accumulating 
	int *pCurrent_ver_accumulator2; //use for vertical accumulator output
	int *pCurrent_ver_blinear_sample; //used for vertical bilinear interpolation 
	int temp_output; 
	int fraction;
	int temp_ver_index; //bugfix002
	int i;
	int debug_temp;
	//if the horizontal scaling factor is too large, for some MCU the veritcal parameters
	//below is not changed at all while for other MCUs it is change
	//it is necessary to record the vertical parameter change for the next row
	int vertical_para_change = 0; //indicating vertical parameter is changed
	int block_ver_partsum_count_Cb; //this one is used when processing a new block
	int block_ver_partsum_count_Cr; //this one is used when processing a new block
	int block_ver_integer_counter_Cb; //used in processing one block, and copy to MCU counter for next MCU row
	int block_ver_fraction_counter_Cb;
	int block_ver_integer_counter_Cr; //used in processing one block, and copy to MCU counter for next MCU row
	int block_ver_fraction_counter_Cr;
	int block_ver_final_output_counter_Cb; //counter for vertital final output
    	int block_ver_final_output_counter_Cr; //counter for vertital final output

	int block_hor_final_output_counter_Cb; //counter for vertital final output
    	int block_hor_final_output_counter_Cr; //counter for vertital final output
    	
	int block_curn_ver_accumulator_Y; //change016
	int block_curn_ver_accumulator_Y2; //change016

	int current_sample_pos=0;
	int x_pos,y_pos,x_mb_pos, y_mb_pos;
	
	//JPEG_ENTRY;
	//hor_final_output_counter and ver_final_output_counter will be used to update output address
	pIDCT_Output = temp_IDCT_buf2;

	if(block_type==0){ //current block is Y
		//manson: now we do transform for this block
		if(!g_imagedec_m33_combine_hw_sw_flag)
		{		
			switch(IDCT_mode_Y){
				case 0:
					idct(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 1:
					idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 2:
					idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 3:
					idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
			}
		}
		//change016
		//block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		//block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		//block_ver_partsum_count_Y = row_ver_partsum_count_Y; //change 018,vertical counters will be updated when preparing next row of MCU
		block_hor_final_output_counter_Y = 0;
		block_ver_final_output_counter_Y = 0;
		//pointer to first row's first sample 
		pOutput_row_first_sample = final_output_buf_Y;
		//pointer to first vertical accumulator
		if(MCU_hor_partsum_first==0){ //for 4:2:0 which has 4 Y blocks
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y1]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y1];
		}
		else{
			pFirst_ver_accumulator = &ver_partsum_Y[block_first_ver_accumulator_Y2]; 
			pFirst_ver_bilinear_sample = &ver_bilinear_Y1[block_first_ver_accumulator_Y2];
		}
		
		flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
		for(ver_counter=0;ver_counter<idct_row_num_Y;ver_counter++){ //vertical counter
			pCurrent_ver_accumulator = pFirst_ver_accumulator;
			flag_final_output_next_row = 0;
				
			if(MCU_hor_partsum_first==0){//bugfix002
				block_hor_integer_counter = hor_integer_counter_Y1;
				temp_hor_partsum_count = hor_partsum_count_Y1; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y1; //counter for horizontal fractional scaling
			}
			else{
				block_hor_integer_counter = hor_integer_counter_Y2;
				temp_hor_partsum_count = hor_partsum_count_Y2; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Y2; //counter for horizontal fractional scaling
			}
			temp_ver_index = MCU_hor_partsum_first+ver_counter;//bugfix002
			for(hor_counter=0;hor_counter<idct_col_num_Y;hor_counter++){ //horizontal counter
				hor_partsum_Y[temp_ver_index] += *pIDCT_Output; //bugfix002//sum for horizontal scaling
				pIDCT_Output++;
				temp_hor_partsum_count++; 
				if(temp_hor_partsum_count>=hor_scale_ratio_Y_int){ //horizontal ready to output one sample
					block_hor_integer_counter++;
					if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
						hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];//bugfix002
					}
					else{ //ready to do horizontal linear interpolation
						fraction = temp_hor_fraction_counter & scale_bits_field;
						temp_output = hor_bilinear_Y1[temp_ver_index]*(scale_fra_total_value-fraction) + hor_partsum_Y[temp_ver_index]*fraction;//bugfix002
						//add to vertical accumulator
						*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
						pCurrent_ver_accumulator++; //point to next vertical sum
						temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_Y_fraction;
						//update hor_bilinear_Y1 since fractional scale factor >= 1.0
						hor_bilinear_Y1[temp_ver_index] = hor_partsum_Y[temp_ver_index];//bugfix002
					}
					hor_partsum_Y[temp_ver_index] = 0; //bugfix002//clear horizontal sum since it has been added to vertical sum
					temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
				}
			}
			pIDCT_Output += 8 - hor_counter;
			if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
				block_ver_partsum_count_Y++;
				vertical_para_change = 1;
#if 0				
				current_sample_pos = hor_final_Y_index_block;
#else
				current_sample_pos = ver_final_Y_index_block;
#endif
				if(block_ver_partsum_count_Y >= ver_scale_ratio_Y_int){//vertical ready to output one sample
#if 0				
					block_hor_final_output_counter_Y = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#else
					block_ver_final_output_counter_Y = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#endif
					pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
					pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
					pOutput_current_sample = pOutput_row_first_sample;
					block_ver_integer_counter_Y++;
					if(block_ver_integer_counter_Y<=(block_ver_fraction_counter_Y>>scale_precision)){//not ready to do interplation yet
						while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
							*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							pCurrent_ver_accumulator2++;
							pCurrent_ver_blinear_sample++;
						}
					}
					else{//ready to do vertical linear interpolation
						fraction = block_ver_fraction_counter_Y & scale_bits_field;
						while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
							temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
							temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
							temp_final_output = temp_output * scale_factor_inverse_2D_Y;
							temp_final_output >>= 15;
							//*pOutput_current_sample = (uchar)clamp(temp_final_output); 
#if 0						
							x_mb_pos = (current_sample_pos>>4);
							x_pos = (current_sample_pos - ((current_sample_pos>>4)<<4));
							y_mb_pos = ((block_ver_final_output_counter_Y+ver_final_Y_index_block)>>4);
							y_pos = (block_ver_final_output_counter_Y+ver_final_Y_index_block) -(((block_ver_final_output_counter_Y+ver_final_Y_index_block)>>4)<<4);
#else
							y_mb_pos = (current_sample_pos>>4);
							y_pos = (current_sample_pos - ((current_sample_pos>>4)<<4));
							x_mb_pos = ((block_hor_final_output_counter_Y+hor_final_Y_index_block)>>4);
							x_pos = (block_hor_final_output_counter_Y+hor_final_Y_index_block) -(((block_hor_final_output_counter_Y+hor_final_Y_index_block)>>4)<<4);							
#endif
							pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + x_pos] = (uchar)clamp(temp_final_output);
#if 0
							UINT32 addr = ((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + x_pos;
							pOutput_current_sample[addr] = (uchar)clamp(temp_final_output);
							addr += (UINT32)pOutput_current_sample;
							if(addr > 0x81000000)
							{
								libc_printf("y_mb %d y_pos %d x_mb %d x_pos %d start %x addr %x\n",y_mb_pos,y_pos,x_mb_pos,x_pos,(UINT32)pOutput_current_sample,addr);
								SDBBP();
							}	
#endif							
							//update ver_bilinear_Y1 since fractional scale factor >= 1.0
							*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
							*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
						//	pOutput_current_sample++;
#if 0						
							current_sample_pos++;
#else
							current_sample_pos--;
#endif
							pCurrent_ver_accumulator2++;
							pCurrent_ver_blinear_sample++;
						}
						block_ver_fraction_counter_Y = block_ver_fraction_counter_Y + ver_scale_ratio_Y_fraction;
						flag_final_output_next_row = 1;
					}
					block_ver_partsum_count_Y = 0; //clear vertical partsum counter
				}
			}
			if(flag_final_output_next_row!=0){ //indicating final output will move to next row
			//	pOutput_row_first_sample += Final_Output_Buf_Stride_Y; //final output move to next row
#if 0			
				block_ver_final_output_counter_Y++;
#else
				block_hor_final_output_counter_Y++;
#endif
			}
		}
		if(MCU_hor_partsum_first==0){//bugfix002
			hor_partsum_count_Y1 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y1 = temp_hor_fraction_counter;
			hor_integer_counter_Y1 = block_hor_integer_counter;
		}
		else{
			hor_partsum_count_Y2 = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Y2 = temp_hor_fraction_counter;
			hor_integer_counter_Y2 = block_hor_integer_counter;
		}
		if(vertical_para_change != 0){
			next_row_ver_partsum_count_Y = block_ver_partsum_count_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_integer_counter_Y = block_ver_integer_counter_Y; //for 4:2:0, the last Y block para is used
			next_row_ver_fraction_counter_Y = block_ver_fraction_counter_Y; //for 4:2:0, the last Y block para is used
			if(MCU_hor_partsum_first==0)
				block_first_ver_accumulator_Y1 = block_first_ver_accumulator_Y1 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			else
				block_first_ver_accumulator_Y2 = block_first_ver_accumulator_Y2 + pCurrent_ver_accumulator - pFirst_ver_accumulator;
		}
		//block_ver_final_output_counter_Y and block_hor_final_output_counter_Y will be use to update output address for next block
	}
	else{ //Cb or Cr
		//manson: now we do transform for this block
		if(!g_imagedec_m33_combine_hw_sw_flag)
		{		
			switch(IDCT_mode_CbCr){
				case 0:
					idct(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 1:
					idct4x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 2:
					idct2x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 3:
					idct1x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 8:
					idct8x4(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 9:
					idct4x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 10:
					idct2x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 17:
					idct8x2(temp_IDCT_buf, temp_IDCT_buf2);
					break;
				case 18:
					idct4x1(temp_IDCT_buf, temp_IDCT_buf2);
					break;
			}
		}
		
	//	libc_printf("IDCT_mode_CbCr:%d\n",IDCT_mode_CbCr);SDBBP();
		if(flag_CbCr_Hor_copy){
			for(i=127;i>=0;i--){
				temp_IDCT_buf2[i] = temp_IDCT_buf2[i>>1];
			}
		}
		if(block_type==1){ //current block is Cb
			//manson: because CbCr may have larger scale factor, 
			//so CbCr still use enhanced bilinear scaling
			block_ver_integer_counter_Cb = row_ver_integer_counter_Cb; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Cb = row_ver_fraction_counter_Cb;
			block_ver_partsum_count_Cb = row_ver_partsum_count_Cb;		
			hor_final_output_counter_Cb = 0;
			block_ver_final_output_counter_Cb = 0;
#if 1			
			ver_final_output_counter_Cb = 0;
			block_hor_final_output_counter_Cb = 0;
#endif
			//pointer to first row's first sample 
			//pOutput_row_first_sample = &final_output_buf_Cb[(ver_final_Cb_index_block*Final_Output_Buf_Stride_CbCr) + hor_final_Cb_index_block];
			pOutput_row_first_sample = final_output_buf_C;
			//pointer to first vertical accumulator
			//manson: if scale factor is too large, some MCU rows may not have final output
			//so we can not take the final output index as the vertical accumulator index (although for small scale factor, they are equivalent)
			pFirst_ver_accumulator = &ver_partsum_Cb[block_first_ver_accumulator_Cb];
			pFirst_ver_bilinear_sample = &ver_bilinear_Cb1[block_first_ver_accumulator_Cb];
			flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
			for(ver_counter=0;ver_counter<idct_row_num_CbCr;ver_counter++){ //vertical counter
				pCurrent_ver_accumulator = pFirst_ver_accumulator;
				flag_final_output_next_row = 0;
				block_hor_integer_counter = hor_integer_counter_Cb;
				temp_hor_partsum_count = hor_partsum_count_Cb; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Cb; //counter for horizontal fractional scaling
		
				for(hor_counter=0;hor_counter<idct_col_num_CbCr;hor_counter++){ //horizontal counter
					hor_partsum_Cb[ver_counter] += *pIDCT_Output; //sum for horizontal scaling
					pIDCT_Output++;
					temp_hor_partsum_count++; 
					if(temp_hor_partsum_count>=hor_scale_ratio_CbCr_int){ //horizontal ready to output one sample
						block_hor_integer_counter++;
						if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
							hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];
						}
						else{ //ready to do horizontal linear interpolation
							fraction = temp_hor_fraction_counter & scale_bits_field;
							temp_output = hor_bilinear_Cb1[ver_counter]*(scale_fra_total_value-fraction) + hor_partsum_Cb[ver_counter]*fraction;
							//add to vertical accumulator
							*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
							pCurrent_ver_accumulator++; //point to next vertical sum
							temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_CbCr_fraction;
							//update hor_bilinear_Cb1 since fractional scale factor >= 1.0
							hor_bilinear_Cb1[ver_counter] = hor_partsum_Cb[ver_counter];
						}
						hor_partsum_Cb[ver_counter] = 0; //clear horizontal sum since it has been added to vertical sum
						temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
					}
				}
				if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
					block_ver_partsum_count_Cb++; 
					vertical_para_change = 1;
#if 0					
					current_sample_pos = hor_final_Cb_index_block;
#else
					current_sample_pos = ver_final_Cb_index_block;
#endif
					if(block_ver_partsum_count_Cb >= ver_scale_ratio_CbCr_int){//vertical ready to output one sample
#if 0					
						hor_final_output_counter_Cb = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#else
						ver_final_output_counter_Cb = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#endif
						pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
						pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
						pOutput_current_sample = pOutput_row_first_sample;
						block_ver_integer_counter_Cb++;
						if(block_ver_integer_counter_Cb<=(block_ver_fraction_counter_Cb>>scale_precision)){//not ready to do interplation yet
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
						}	
						else{//ready to do vertical linear interpolation
							fraction = block_ver_fraction_counter_Cb & scale_bits_field;
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
								temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
								temp_final_output = temp_output * scale_factor_inverse_2D_CbCr;
								temp_final_output >>= 15;

#if 0
								x_mb_pos = (current_sample_pos>>3);
								x_pos = (current_sample_pos - ((current_sample_pos>>3)<<3));
								y_mb_pos = ((block_ver_final_output_counter_Cb+ver_final_Cb_index_block)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = (block_ver_final_output_counter_Cb+ver_final_Cb_index_block) - (((block_ver_final_output_counter_Cb+ver_final_Cb_index_block)>>4)<<4);
#else
								x_mb_pos = ((block_hor_final_output_counter_Cb+hor_final_Cb_index_block)>>3);
								x_pos = ((block_hor_final_output_counter_Cb+hor_final_Cb_index_block) - (((block_hor_final_output_counter_Cb+hor_final_Cb_index_block)>>3)<<3));
								y_mb_pos = ((current_sample_pos)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = current_sample_pos - ((current_sample_pos>>4)<<4);
#endif	
							
							//	libc_printf("x_:%d y_:%d\n",current_sample_pos,(block_ver_final_output_counter_Cb+ver_final_Cb_index_block));
							//	libc_printf("pos1:%x\n", y_mb_pos*45*16*16 + (x_mb_pos<<8) + y_pos*16 + x_pos*2 + 1);
								pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8)+ (x_mb_pos<<8) + (y_pos<<4) + (x_pos<<1) + 1] = (uchar)clamp(temp_final_output); 

								//update ver_bilinear_Cb1 since fractional scale factor >= 1.0
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//	pOutput_current_sample++;
#if 0							
								current_sample_pos++;
#else
								current_sample_pos--;
#endif
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
							block_ver_fraction_counter_Cb = block_ver_fraction_counter_Cb + ver_scale_ratio_CbCr_fraction;
							flag_final_output_next_row = 1;
						}
						block_ver_partsum_count_Cb = 0; //clear vertical partsum counter
					}
				}
				if(flag_final_output_next_row!=0){ //indicating final output will move to next row
				//	pOutput_row_first_sample += Final_Output_Buf_Stride_CbCr; //final output move to next row
#if 0				
					block_ver_final_output_counter_Cb++;
#else
					block_hor_final_output_counter_Cb++;
#endif
				}
			}
			hor_partsum_count_Cb = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Cb = temp_hor_fraction_counter;
			hor_integer_counter_Cb = block_hor_integer_counter;
			if(vertical_para_change != 0){
				next_row_ver_partsum_count_Cb = block_ver_partsum_count_Cb;
				next_row_ver_integer_counter_Cb=block_ver_integer_counter_Cb;
				next_row_ver_fraction_counter_Cb = block_ver_fraction_counter_Cb;
#if 0				
				row_ver_final_output_counter_Cb = block_ver_final_output_counter_Cb; //counter for vertital final output
#else
				row_hor_final_output_counter_Cb = block_hor_final_output_counter_Cb; 	
#endif
				block_first_ver_accumulator_Cb = block_first_ver_accumulator_Cb + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			}
			//if(block_ver_partsum_count_Cb != row_ver_partsum_count_Cb)
			//	next_row_ver_partsum_count_Cb = block_ver_partsum_count_Cb;
			//block_ver_final_output_counter_Cb and hor_final_output_counter_Cb will be use to update output address for next block
		}
		else{//current block is Cr
			//for debug
			block_ver_integer_counter_Cr = row_ver_integer_counter_Cr;//vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Cr = row_ver_fraction_counter_Cr;
			block_ver_partsum_count_Cr = row_ver_partsum_count_Cr;
			hor_final_output_counter_Cr = 0;
			block_ver_final_output_counter_Cr = 0;
#if 1			
			ver_final_output_counter_Cr = 0;
			block_hor_final_output_counter_Cr = 0;
#endif		
			//pointer to first row's first sample 
		//	pOutput_row_first_sample = &final_output_buf_Cr[(ver_final_Cr_index_block*Final_Output_Buf_Stride_CbCr) + hor_final_Cr_index_block];
			pOutput_row_first_sample = final_output_buf_C;
			//pointer to first vertical accumulator
			pFirst_ver_accumulator = &ver_partsum_Cr[block_first_ver_accumulator_Cr];
			pFirst_ver_bilinear_sample = &ver_bilinear_Cr1[block_first_ver_accumulator_Cr];
			
			flag_final_output_next_row = 0; //if this flag is set in the loop, move to the next row of output
			for(ver_counter=0;ver_counter<idct_row_num_CbCr;ver_counter++){ //vertical counter
				pCurrent_ver_accumulator = pFirst_ver_accumulator;
				flag_final_output_next_row = 0;
				block_hor_integer_counter = hor_integer_counter_Cr;
				temp_hor_partsum_count = hor_partsum_count_Cr; //horizotal partsum counter is used row by row
				temp_hor_fraction_counter = hor_fraction_counter_Cr; //counter for horizontal fractional scaling
		
				for(hor_counter=0;hor_counter<idct_col_num_CbCr;hor_counter++){ //horizontal counter
					hor_partsum_Cr[ver_counter] += *pIDCT_Output; //sum for horizontal scaling
					pIDCT_Output++;
					temp_hor_partsum_count++; 
					if(temp_hor_partsum_count>=hor_scale_ratio_CbCr_int){ //horizontal ready to output one sample
						block_hor_integer_counter++;
						if(block_hor_integer_counter<=(temp_hor_fraction_counter>>scale_precision)){//not ready for linear interpolation
							hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter];
						}
						else{ //ready to do horizontal linear interpolation
							fraction = temp_hor_fraction_counter & scale_bits_field;
							temp_output = hor_bilinear_Cr1[ver_counter]*(scale_fra_total_value-fraction) + hor_partsum_Cr[ver_counter]*fraction;
							//add to vertical accumulator
							*pCurrent_ver_accumulator += temp_output; //sum for vertical scaling
							pCurrent_ver_accumulator++; //point to next vertical sum
							temp_hor_fraction_counter = temp_hor_fraction_counter + hor_scale_ratio_CbCr_fraction;
							//update hor_bilinear_Cr1 since fractional scale factor >= 1.0
							hor_bilinear_Cr1[ver_counter] = hor_partsum_Cr[ver_counter]; //bugfix001
						}
						hor_partsum_Cr[ver_counter] = 0; //clear horizontal sum since it has been added to vertical sum
						temp_hor_partsum_count = 0; //indicating horizontal partsum is cleared
					}
				}
				if(pCurrent_ver_accumulator!=pFirst_ver_accumulator){ //indicating vertical partsum add new data
					block_ver_partsum_count_Cr++; 
					vertical_para_change = 1;
#if 0					
					current_sample_pos = hor_final_Cr_index_block;
#else
					current_sample_pos = ver_final_Cr_index_block;
#endif
					if(block_ver_partsum_count_Cr >= ver_scale_ratio_CbCr_int){//vertical ready to output one sample
#if 0					
						hor_final_output_counter_Cr = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#else
						ver_final_output_counter_Cr = pCurrent_ver_accumulator - pFirst_ver_accumulator; //unfortunately we have to compute it multiple times
#endif
						pCurrent_ver_accumulator2 = pFirst_ver_accumulator;
						pCurrent_ver_blinear_sample = pFirst_ver_bilinear_sample;
						pOutput_current_sample = pOutput_row_first_sample;
						block_ver_integer_counter_Cr++;
						if(block_ver_integer_counter_Cr<=(block_ver_fraction_counter_Cr>>scale_precision)){//not ready to do interplation yet
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
						}
						else{//ready to do vertical linear interpolation
							fraction = block_ver_fraction_counter_Cr & scale_bits_field;
							while(pCurrent_ver_accumulator2!=pCurrent_ver_accumulator){
								temp_output = (*pCurrent_ver_blinear_sample)*(scale_fra_total_value-fraction) + (*pCurrent_ver_accumulator2)*fraction;
								temp_output = temp_output>>scale_precisionX2;//>> 16 to discard the fractional bits
								temp_final_output = temp_output * scale_factor_inverse_2D_CbCr;
								temp_final_output >>= 15;
							//	libc_printf("current_sample_pos:%d\n",current_sample_pos);
							//	libc_printf("block_ver_final_output_counter_Cr:%d\n",block_ver_final_output_counter_Cr);
							//	libc_printf("ver_final_Cr_index_block:%d\n",ver_final_Cr_index_block);
#if 0							
								x_mb_pos = (current_sample_pos>>3);
								x_pos = (current_sample_pos - ((current_sample_pos>>3)<<3));
								y_mb_pos = ((block_ver_final_output_counter_Cr+ver_final_Cr_index_block)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = (block_ver_final_output_counter_Cr+ver_final_Cr_index_block) - (((block_ver_final_output_counter_Cr+ver_final_Cr_index_block)>>4)<<4);
#else
								x_mb_pos = ((block_hor_final_output_counter_Cr+hor_final_Cr_index_block)>>3);
								x_pos = ((block_hor_final_output_counter_Cr+hor_final_Cr_index_block) - (((block_hor_final_output_counter_Cr+hor_final_Cr_index_block)>>3)<<3));
								y_mb_pos = ((current_sample_pos)>>4);
							//	y_mb_pos = (y_mb_pos>>1);
								y_pos = current_sample_pos - ((current_sample_pos>>4)<<4);
#endif
//
							//	libc_printf("x_pos:%d y_pos:%d\n",x_pos,y_pos);

																					//	libc_printf("pos2:%x\n", y_mb_pos*45*16*16 + (x_mb_pos<<8) + y_pos*16 + x_pos*2);
								pOutput_current_sample[((y_mb_pos*g_jpeg_dst_stride)<<8) + (x_mb_pos<<8) + (y_pos<<4) + (x_pos<<1)] = (uchar)clamp(temp_final_output); 

	//							libc_printf("sample:%x\n",(uchar)clamp(temp_final_output));
							//	*pOutput_current_sample = (uchar)clamp(temp_final_output); 

	
								//update ver_bilinear_Cr1 since fractional scale factor >= 1.0
								*pCurrent_ver_blinear_sample = *pCurrent_ver_accumulator2;
								*pCurrent_ver_accumulator2 = 0; //clear vertical partsum accumulator
							//	pOutput_current_sample++;
#if 0							
								current_sample_pos++;
#else
								current_sample_pos--;
#endif
								pCurrent_ver_accumulator2++;
								pCurrent_ver_blinear_sample++;
							}
							block_ver_fraction_counter_Cr = block_ver_fraction_counter_Cr + ver_scale_ratio_CbCr_fraction;
							flag_final_output_next_row = 1;
						}
						block_ver_partsum_count_Cr = 0; //clear vertical partsum counter
					}	
				}
				if(flag_final_output_next_row!=0){ //indicating final output will move to next row
				//	pOutput_row_first_sample += Final_Output_Buf_Stride_CbCr; //final output move to next row
#if 0				
					block_ver_final_output_counter_Cr++;
#else
					block_hor_final_output_counter_Cr++;
#endif
				}
			}
			hor_partsum_count_Cr = temp_hor_partsum_count; //update horizontal partsum counter for next block
			hor_fraction_counter_Cr = temp_hor_fraction_counter;
			hor_integer_counter_Cr = block_hor_integer_counter;
			if(vertical_para_change != 0){
				next_row_ver_partsum_count_Cr = block_ver_partsum_count_Cr;
				next_row_ver_integer_counter_Cr=block_ver_integer_counter_Cr;
				next_row_ver_fraction_counter_Cr = block_ver_fraction_counter_Cr;
#if 0				
				row_ver_final_output_counter_Cr = block_ver_final_output_counter_Cr; //counter for vertital final output
#else
				row_hor_final_output_counter_Cr = block_hor_final_output_counter_Cr; //counter for vertital final output
#endif
				block_first_ver_accumulator_Cr = block_first_ver_accumulator_Cr + pCurrent_ver_accumulator - pFirst_ver_accumulator;
			}
			//if(block_ver_partsum_count_Cr != row_ver_partsum_count_Cr)
			//	next_row_ver_partsum_count_Cr = block_ver_partsum_count_Cr;
			//block_ver_final_output_counter_Cr and hor_final_output_counter_Cr will be use to update output address for next block
		}
	}
	//JPEG_EXIT;
}

//some functions added by Manson
//------------------------------------------------------------------------------
static void IDCT_Scaling_ORI(pImagedec_hdl phdl, int Y_only)
{
	int temp_height;
	int extra_columns; //change014
/*	
	//use this debug code to verify 4x4 IDCT
	int debug_IDCT_buf1[64];
	int debug_IDCT_buf2[64];
	int i;

	for(i=0;i<64;i++)
		debug_IDCT_buf1[i] = *(pInputData+i);
	idct(pInputData, temp_IDCT_buf2);
	idct4x1(debug_IDCT_buf1, debug_IDCT_buf2);
	//for(i=0;i<16;i++){ //4x4
	//for(i=0;i<4;i++){ //2x2
	//for(i=0;i<32;i++){ //8x4
	//for(i=0;i<8;i++){ //4x2
	//for(i=0;i<2;i++){ //2x1
	//for(i=0;i<16;i++){ //8x2
	for(i=0;i<4;i++){ //4x1
		if(debug_IDCT_buf2[i] != temp_IDCT_buf2[i])
			debug_counter++;
	}
	debug_counter2++;
*/

	MCU_block_counter++;
	switch (phdl->decoder.out_type){
	case JPGD_YH2V2: //4:2:0
		if(MCU_block_counter==5){//libc_printf("MCU_block_counter:%d\n",MCU_block_counter); //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==6){//libc_printf("MCU_block_counter:%d\n",MCU_block_counter); //Cb //bugfix002
			//manson: use idct_col_num_CbCr of MCU_block_counter==5
			IDCT_BilScaling_Routine(1);
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		//	SDBBP();
		}
		else{
		
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y += (idct_col_num_Y_ForMost << 1); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block2 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 1)-extra_columns + 1)>>1;
					//jpeg_printf("extra <%d>idct_col_1 <%d>2<%d>\n",extra_columns,idct_col_num_Y_block1,idct_col_num_Y_block2);					
				}
				else{
						flag_is_last_hor_MCU = 0;
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
					}

				idct_col_num_Y = idct_col_num_Y_block1; //change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;

				//change016; for supporting 4:2:0
				ver_integer_counter_Y1 = row_ver_integer_counter_Y; //change 016
				ver_integer_counter_Y2 = row_ver_integer_counter_Y; //for support 4:2:0, block 1-3 share one counter, block 2-4 share another
				ver_fraction_counter_Y1 = row_ver_fraction_counter_Y; 
				ver_fraction_counter_Y2 = row_ver_fraction_counter_Y; 
				ver_partsum_count_Y1 = row_ver_partsum_count_Y; //change018
				ver_partsum_count_Y2 = row_ver_partsum_count_Y; //change018

				block_ver_integer_counter_Y = ver_integer_counter_Y1; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y1;
				block_ver_partsum_count_Y = ver_partsum_count_Y1;
				
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y1 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y1 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y1 = block_ver_partsum_count_Y;

				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
			//	libc_printf("type<1>hor_final_Y_index_MCU:%d block_hor_final_output_counter_Y:%d\n",hor_final_Y_index_MCU,block_hor_final_output_counter_Y);
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
				first_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
				break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;

				block_ver_integer_counter_Y = ver_integer_counter_Y2; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y2;
				block_ver_partsum_count_Y = ver_partsum_count_Y2;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y2 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y2 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y2 = block_ver_partsum_count_Y;

				hor_final_Y_index_MCU_tophalf = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change014
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU 
				//manson: if the picture size is very large and vertical, sometimes 
				//for some blocks block_ver_final_output_counter_Y can be zero
				//the block3's output will be written to a wrong position
				//to avoid this case, use the non-zero vertical counter of block 1 or 2
				//there is a very special case that the last MCU may totally has no output
				//because the picture size is not integer of MCU size. So we only update 
				//MCU_output_ver_count when the MCU is not the last one.
				//manson: although the last block has less column, it is still possible
				//that the first block doesn't have output while the last block has
				if(block_ver_final_output_counter_Y!=0){ //change016
					ver_final_Y_index_block = ver_final_Y_index_MCU+block_ver_final_output_counter_Y; //vertical is increased  
				}
				else{
					ver_final_Y_index_block = ver_final_Y_index_MCU+first_ver_final_output_counter_Y; //vertical is increased  
				}
				//change018
				if(block_ver_final_output_counter_Y!=0){//change017
					MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_ver_count = first_ver_final_output_counter_Y; //bugfix002
				}
			break;
			case 3: //third Y block
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;

				block_ver_integer_counter_Y = ver_integer_counter_Y1; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y1;
				block_ver_partsum_count_Y = ver_partsum_count_Y1;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y1 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y1 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y1 = block_ver_partsum_count_Y;
				
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				third_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
				//ver_final_Y_index_block vertical is the same as block 3
			break;
			case 4: //fourth Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;
				
				block_ver_integer_counter_Y = ver_integer_counter_Y2; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y2;
				block_ver_partsum_count_Y = ver_partsum_count_Y2;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y2 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y2 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y2 = block_ver_partsum_count_Y;

				//change018 manson: if the picture size is very large, top half or bottom half may have 0 output
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change018
				if(hor_final_Y_index_block > hor_final_Y_index_MCU_tophalf)
					hor_final_Y_index_MCU = hor_final_Y_index_block;
				else
					hor_final_Y_index_MCU = hor_final_Y_index_MCU_tophalf; //change014
				//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
				//MCU final output vertical address will be updated after one row of MCU processing
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 

				//manson: for the last MCU line, if the picture size is not integer times of MCU size
				//we shall take the top half of the MCU's block_hor_final_output_counter_Y
				//if the picture size is very large, for example 8000x6000, the first
				//16x16 MCU may not have output. So we can not always take the first MCU output counter for the whole MCU row
				if(block_ver_final_output_counter_Y!=0){
					MCU_output_ver_count = MCU_output_ver_count + block_ver_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_ver_count = MCU_output_ver_count + third_ver_final_output_counter_Y;
				}
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH2V1: //4:2:2
		if(MCU_block_counter==3){ //Cr
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
			//ver_final_Cr_index_block //vertical is not changed
		}
		else if(MCU_block_counter==4){ //Cb
			IDCT_BilScaling_Routine(1);
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
			//ver_final_Cb_index_block //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			//final output address for this block has been updated when the previous block is finished
			//update final output index for next block
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 1); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block2 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 1)-extra_columns + 1)>>1;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
				}
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0; //bugfix002

				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0);
				first_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
				
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
				break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0; //bugfix002

				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0); //change014
				if(flag_is_last_hor_MCU==0){
					hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					if(block_ver_final_output_counter_Y!=0)
						MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
					else
						MCU_output_ver_count = first_ver_final_output_counter_Y;
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH1V2: //vertically decimated by 2, horizontal the same as Y
		//I don't want to spend my time to write a 4x8 IDCT
		//but it is not necessary to optimize since it is not popular anyway
		if(MCU_block_counter==3){ //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==4){ //Cb //bugfix002
			IDCT_BilScaling_Routine(1);
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
					idct_col_num_CbCr_ForLast = idct_col_num_Y_ForMost-extra_columns;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
				}
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;
				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0);
				hor_final_Y_index_MCU_tophalf = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //change014
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU 
				ver_final_Y_index_block = ver_final_Y_index_MCU+block_ver_final_output_counter_Y; //vertical is increased  
				//change017
				MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
			break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;
				//block_ver_integer_counter_Y and block_ver_fraction_counter_Y and block_ver_partsum_count_Y are updated in first block
				IDCT_BilScaling_Routine(0);
				if(flag_is_last_hor_MCU==0){
					//change018
					hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change018
					if(hor_final_Y_index_block > hor_final_Y_index_MCU_tophalf)
						hor_final_Y_index_MCU = hor_final_Y_index_block;
					else
						hor_final_Y_index_MCU = hor_final_Y_index_MCU_tophalf; //change014
					//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_ver_count = MCU_output_ver_count + block_ver_final_output_counter_Y; //change017
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH4V1: //4:1:1
		if(MCU_block_counter==5){ //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==6){ //Cb //bugfix002
			IDCT_BilScaling_Routine(1);
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			//final output address for this block has been updated when the previous block is finished
			idct_row_num_Y = idct_row_num_Y1;
			block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
			block_ver_partsum_count_Y = row_ver_partsum_count_Y;
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 2); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > (idct_col_num_Y_ForMost*3)){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 2) - extra_columns;
						idct_col_num_Y_block2 = 0;
						idct_col_num_Y_block3 = 0;
						idct_col_num_Y_block4 = 0;
					}
					else if(extra_columns > (idct_col_num_Y_ForMost<<1)){
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = (idct_col_num_Y_ForMost * 3) - extra_columns;
						idct_col_num_Y_block3 = 0;
						idct_col_num_Y_block4 = 0;
					}
					else if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block3 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block4 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block3 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block4 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 2)-extra_columns + 3)>>2;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block3 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block4 = idct_col_num_Y_ForMost;
				}
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				IDCT_BilScaling_Routine(0);
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
				break;
			case 2: //second Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				IDCT_BilScaling_Routine(0);
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical not change
			break;
			case 3: //third Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block3;//change014
				IDCT_BilScaling_Routine(0);
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical not change
			break;
			case 4: //fourth Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block4;//change014
				IDCT_BilScaling_Routine(0);
				if(flag_is_last_hor_MCU==0){
					hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH1V1: //4:4:4
		if(MCU_block_counter==2){ //Cr
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
			//ver_final_Cr_index_block //vertical is not changed
		}
		else if(MCU_block_counter==3){ //Cb
			IDCT_BilScaling_Routine(1);
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
			//ver_final_Cb_index_block //vertical is not change 
			MCU_block_counter = 0;
		}
		else if(MCU_block_counter==1){ //Y
			//change014
			decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
			if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
				flag_is_last_hor_MCU = 1;
				extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
				idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
				idct_col_num_CbCr_ForLast = (idct_col_num_Y_block1 + 1)>>1;//change019
			}
			else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
				}
			idct_col_num_Y = idct_col_num_Y_block1;//change014
			idct_row_num_Y = idct_row_num_Y1;
			MCU_hor_partsum_first = 0; //bugfix002
			block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
			block_ver_partsum_count_Y = row_ver_partsum_count_Y;
			IDCT_BilScaling_Routine(0);
			if(flag_is_last_hor_MCU==0){
				//update final output index for next block
				hor_final_Y_index_block += block_hor_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block //vertical is not changed
				//manson: MCU_output_ver_count has been assigned default value 0 already
				//this is needed because if the picture size is very large, MCU_output_ver_count may really has 0 value
				MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
			}
			if(Y_only!=0) //for progressive JPEG component Y
				MCU_block_counter = 0;
		}
		break;
	case JPGD_GRAYSCALE:
		//change014
		decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
		if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
			flag_is_last_hor_MCU = 1;
			extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
			idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
			idct_col_num_CbCr_ForLast = idct_col_num_Y_ForMost-extra_columns;
		}
		else{
			flag_is_last_hor_MCU = 0;
			idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
		}
		idct_col_num_Y = idct_col_num_Y_block1;//change014
		idct_row_num_Y = idct_row_num_Y1;
		MCU_hor_partsum_first = 0; //bugfix002
		block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		block_ver_partsum_count_Y = row_ver_partsum_count_Y;
		IDCT_BilScaling_Routine(0);
		//update final output index for next block
		hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
		//ver_final_Y_index_block //vertical is not changed
		MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
		if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
			max_MCU_output_ver_count_Y = MCU_output_ver_count;
		break;
	}
}

static void IDCT_Scaling_90A(pImagedec_hdl phdl, int Y_only)
{
	int temp_height;
	int extra_columns; //change014
/*	
	//use this debug code to verify 4x4 IDCT
	int debug_IDCT_buf1[64];
	int debug_IDCT_buf2[64];
	int i;

	for(i=0;i<64;i++)
		debug_IDCT_buf1[i] = *(pInputData+i);
	idct(pInputData, temp_IDCT_buf2);
	idct4x1(debug_IDCT_buf1, debug_IDCT_buf2);
	//for(i=0;i<16;i++){ //4x4
	//for(i=0;i<4;i++){ //2x2
	//for(i=0;i<32;i++){ //8x4
	//for(i=0;i<8;i++){ //4x2
	//for(i=0;i<2;i++){ //2x1
	//for(i=0;i<16;i++){ //8x2
	for(i=0;i<4;i++){ //4x1
		if(debug_IDCT_buf2[i] != temp_IDCT_buf2[i])
			debug_counter++;
	}
	debug_counter2++;
*/
	//JPEG_ENTRY;
	MCU_block_counter++;
	switch (phdl->decoder.out_type){
	case JPGD_YH2V2: //4:2:0
		if(MCU_block_counter==5){//libc_printf("MCU_block_counter:%d\n",MCU_block_counter); //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block += ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==6){//libc_printf("MCU_block_counter:%d\n",MCU_block_counter); //Cb //bugfix002
			//manson: use idct_col_num_CbCr of MCU_block_counter==5
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block += ver_final_output_counter_Cb; //horizontal is increased  
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		//	SDBBP();
		}
		else{
		
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 1); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block2 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 1)-extra_columns)>>1;
					jpeg_printf("extra <%d>idct_col_1 <%d>2<%d>\n",extra_columns,idct_col_num_Y_block1,idct_col_num_Y_block2);					
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
				}

				idct_col_num_Y = idct_col_num_Y_block1; //change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;

				//change016; for supporting 4:2:0
				ver_integer_counter_Y1 = row_ver_integer_counter_Y; //change 016
				ver_integer_counter_Y2 = row_ver_integer_counter_Y; //for support 4:2:0, block 1-3 share one counter, block 2-4 share another
				ver_fraction_counter_Y1 = row_ver_fraction_counter_Y; 
				ver_fraction_counter_Y2 = row_ver_fraction_counter_Y; 
				ver_partsum_count_Y1 = row_ver_partsum_count_Y; //change018
				ver_partsum_count_Y2 = row_ver_partsum_count_Y; //change018

				block_ver_integer_counter_Y = ver_integer_counter_Y1; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y1;
				block_ver_partsum_count_Y = ver_partsum_count_Y1;
				
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y1 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y1 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y1 = block_ver_partsum_count_Y;

#if 0
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
			//	libc_printf("type<1>hor_final_Y_index_MCU:%d block_hor_final_output_counter_Y:%d\n",hor_final_Y_index_MCU,block_hor_final_output_counter_Y);
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
				first_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
#else
				ver_final_Y_index_block = ver_final_Y_index_MCU + block_ver_final_output_counter_Y;
				hor_final_Y_index_block = hor_final_Y_index_MCU;
				first_hor_final_output_counter_Y = block_hor_final_output_counter_Y;
#endif				
				break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;

				block_ver_integer_counter_Y = ver_integer_counter_Y2; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y2;
				block_ver_partsum_count_Y = ver_partsum_count_Y2;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y2 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y2 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y2 = block_ver_partsum_count_Y;

#if 0
				hor_final_Y_index_MCU_tophalf = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change014
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU 
				//manson: if the picture size is very large and vertical, sometimes 
				//for some blocks block_ver_final_output_counter_Y can be zero
				//the block3's output will be written to a wrong position
				//to avoid this case, use the non-zero vertical counter of block 1 or 2
				//there is a very special case that the last MCU may totally has no output
				//because the picture size is not integer of MCU size. So we only update 
				//MCU_output_ver_count when the MCU is not the last one.
				//manson: although the last block has less column, it is still possible
				//that the first block doesn't have output while the last block has
				if(block_ver_final_output_counter_Y!=0){ //change016
					ver_final_Y_index_block = ver_final_Y_index_MCU+block_ver_final_output_counter_Y; //vertical is increased  
				}
				else{
					ver_final_Y_index_block = ver_final_Y_index_MCU+first_ver_final_output_counter_Y; //vertical is increased  
				}
				//change018
				if(block_ver_final_output_counter_Y!=0){//change017
					MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_ver_count = first_ver_final_output_counter_Y; //bugfix002
				}
#else
				ver_final_Y_index_MCU_tophalf = ver_final_Y_index_block + block_ver_final_output_counter_Y; //change014
				ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU 
				//manson: if the picture size is very large and vertical, sometimes 
				//for some blocks block_ver_final_output_counter_Y can be zero
				//the block3's output will be written to a wrong position
				//to avoid this case, use the non-zero vertical counter of block 1 or 2
				//there is a very special case that the last MCU may totally has no output
				//because the picture size is not integer of MCU size. So we only update 
				//MCU_output_ver_count when the MCU is not the last one.
				//manson: although the last block has less column, it is still possible
				//that the first block doesn't have output while the last block has
				if(block_hor_final_output_counter_Y!=0){ //change016
					hor_final_Y_index_block = hor_final_Y_index_MCU - block_hor_final_output_counter_Y; //vertical is increased  
				}
				else{
					hor_final_Y_index_block = hor_final_Y_index_MCU - first_hor_final_output_counter_Y; //vertical is increased  
				}
				//change018
				if(block_hor_final_output_counter_Y!=0){//change017
					MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_hor_count = first_hor_final_output_counter_Y; //bugfix002
				}
#endif				
				break;
			case 3: //third Y block
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;

				block_ver_integer_counter_Y = ver_integer_counter_Y1; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y1;
				block_ver_partsum_count_Y = ver_partsum_count_Y1;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y1 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y1 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y1 = block_ver_partsum_count_Y;

#if 0				
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				third_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
				//ver_final_Y_index_block vertical is the same as block 3
#else
				ver_final_Y_index_block = ver_final_Y_index_MCU  + block_ver_final_output_counter_Y; //horizontal is increased  
				third_hor_final_output_counter_Y = block_hor_final_output_counter_Y;
#endif
				break;
			case 4: //fourth Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;
				
				block_ver_integer_counter_Y = ver_integer_counter_Y2; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y2;
				block_ver_partsum_count_Y = ver_partsum_count_Y2;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y2 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y2 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y2 = block_ver_partsum_count_Y;

#if 0
				//change018 manson: if the picture size is very large, top half or bottom half may have 0 output
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change018
				if(hor_final_Y_index_block > hor_final_Y_index_MCU_tophalf)
					hor_final_Y_index_MCU = hor_final_Y_index_block;
				else
					hor_final_Y_index_MCU = hor_final_Y_index_MCU_tophalf; //change014
				//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
				//MCU final output vertical address will be updated after one row of MCU processing
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 

				//manson: for the last MCU line, if the picture size is not integer times of MCU size
				//we shall take the top half of the MCU's block_hor_final_output_counter_Y
				//if the picture size is very large, for example 8000x6000, the first
				//16x16 MCU may not have output. So we can not always take the first MCU output counter for the whole MCU row
				if(block_ver_final_output_counter_Y!=0){
					MCU_output_ver_count = MCU_output_ver_count + block_ver_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_ver_count = MCU_output_ver_count + third_ver_final_output_counter_Y;
				}
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
				ver_final_Y_index_block += block_ver_final_output_counter_Y; //change018
				if(ver_final_Y_index_block > ver_final_Y_index_MCU_tophalf)
					ver_final_Y_index_MCU = ver_final_Y_index_block;
				else
					ver_final_Y_index_MCU = ver_final_Y_index_MCU_tophalf; //change014
					
				//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
				//MCU final output vertical address will be updated after one row of MCU processing
				ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU  
				hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 

				//manson: for the last MCU line, if the picture size is not integer times of MCU size
				//we shall take the top half of the MCU's block_hor_final_output_counter_Y
				//if the picture size is very large, for example 8000x6000, the first
				//16x16 MCU may not have output. So we can not always take the first MCU output counter for the whole MCU row
				if(block_hor_final_output_counter_Y!=0){
					MCU_output_hor_count += block_hor_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_hor_count += third_hor_final_output_counter_Y;
				}
				if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
					max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH2V1: //4:2:2
		if(MCU_block_counter==3){ //Cr
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block += ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block //vertical is not changed
		}
		else if(MCU_block_counter==4){ //Cb
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block += ver_final_output_counter_Cb; //horizontal is increased  
#endif			
			//ver_final_Cb_index_block //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			//final output address for this block has been updated when the previous block is finished
			//update final output index for next block
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 1); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block2 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 1)-extra_columns)>>1;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
				}
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0; //bugfix002

				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0);
#if 0				
				first_ver_final_output_counter_Y = block_ver_final_output_counter_Y;				
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
#else
				first_hor_final_output_counter_Y = block_hor_final_output_counter_Y;				
				ver_final_Y_index_block += block_ver_final_output_counter_Y; //horizontal is increased  
				hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
#endif
				break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0; //bugfix002

				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0); //change014
				if(flag_is_last_hor_MCU==0){

#if 0		
					hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					if(block_ver_final_output_counter_Y!=0)
						MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
					else
						MCU_output_ver_count = first_ver_final_output_counter_Y;
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
					ver_final_Y_index_MCU = ver_final_Y_index_block + block_ver_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU  
					hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
					if(block_hor_final_output_counter_Y!=0)
						MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
					else
						MCU_output_hor_count = first_hor_final_output_counter_Y;
					if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
						max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH1V2: //vertically decimated by 2, horizontal the same as Y
		//I don't want to spend my time to write a 4x8 IDCT
		//but it is not necessary to optimize since it is not popular anyway
		if(MCU_block_counter==3){ //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block += ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==4){ //Cb //bugfix002
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block += ver_final_output_counter_Cb; //horizontal is increased  
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
					idct_col_num_CbCr_ForLast = idct_col_num_Y_ForMost-extra_columns;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
				}
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;
				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0);
#if 0				
				hor_final_Y_index_MCU_tophalf = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //change014
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU 
				ver_final_Y_index_block = ver_final_Y_index_MCU+block_ver_final_output_counter_Y; //vertical is increased  
				//change017
				MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
				ver_final_Y_index_MCU_tophalf = ver_final_Y_index_MCU + block_ver_final_output_counter_Y; //change014
				ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU 
				hor_final_Y_index_block = hor_final_Y_index_MCU - block_hor_final_output_counter_Y; //vertical is increased  
				//change017
				MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
				if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
					max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				
			break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;
				//block_ver_integer_counter_Y and block_ver_fraction_counter_Y and block_ver_partsum_count_Y are updated in first block
				IDCT_BilScaling_Routine(0);
				if(flag_is_last_hor_MCU==0){
					//change018
#if	0				
					hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change018
					if(hor_final_Y_index_block > hor_final_Y_index_MCU_tophalf)
						hor_final_Y_index_MCU = hor_final_Y_index_block;
					else
						hor_final_Y_index_MCU = hor_final_Y_index_MCU_tophalf; //change014
					//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_ver_count = MCU_output_ver_count + block_ver_final_output_counter_Y; //change017
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
					ver_final_Y_index_block += block_ver_final_output_counter_Y; //change018
					if(ver_final_Y_index_block > ver_final_Y_index_MCU_tophalf)
						ver_final_Y_index_MCU = ver_final_Y_index_block;
					else
						ver_final_Y_index_MCU = ver_final_Y_index_MCU_tophalf; //change014
					//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU  
					hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_hor_count += block_hor_final_output_counter_Y; //change017
					if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
						max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH4V1: //4:1:1
		if(MCU_block_counter==5){ //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block += ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==6){ //Cb //bugfix002
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block += ver_final_output_counter_Cb; //horizontal is increased  
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			
			MCU_block_counter = 0;
		}
		else{
			//final output address for this block has been updated when the previous block is finished
			idct_row_num_Y = idct_row_num_Y1;
			block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
			block_ver_partsum_count_Y = row_ver_partsum_count_Y;
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 2); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > (idct_col_num_Y_ForMost*3)){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 2) - extra_columns;
						idct_col_num_Y_block2 = 0;
						idct_col_num_Y_block3 = 0;
						idct_col_num_Y_block4 = 0;
					}
					else if(extra_columns > (idct_col_num_Y_ForMost<<1)){
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = (idct_col_num_Y_ForMost * 3) - extra_columns;
						idct_col_num_Y_block3 = 0;
						idct_col_num_Y_block4 = 0;
					}
					else if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block3 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block4 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block3 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block4 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 2)-extra_columns)>>2;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block3 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block4 = idct_col_num_Y_ForMost;
				}
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				IDCT_BilScaling_Routine(0);
#if 0				
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
#else
				ver_final_Y_index_block = ver_final_Y_index_MCU + block_ver_final_output_counter_Y; //horizontal is increased  
				hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
#endif
				break;
			case 2: //second Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				IDCT_BilScaling_Routine(0);
#if 0				
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
#else
				ver_final_Y_index_block += block_ver_final_output_counter_Y; //horizontal is increased  
#endif
				//ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical not change
			break;
			case 3: //third Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block3;//change014
				IDCT_BilScaling_Routine(0);
#if 0				
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
#else
				ver_final_Y_index_block += block_ver_final_output_counter_Y; 
#endif
				//ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical not change
			break;
			case 4: //fourth Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block4;//change014
				IDCT_BilScaling_Routine(0);
				if(flag_is_last_hor_MCU==0){
#if 0					
					hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
					ver_final_Y_index_MCU = ver_final_Y_index_block + block_ver_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU  
					hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
					if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
						max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH1V1: //4:4:4
		if(MCU_block_counter==2){ //Cr
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block += ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==3){ //Cb
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block += ver_final_output_counter_Cb; //horizontal is increased  
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else if(MCU_block_counter==1){ //Y
			//change014
			decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
			if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
				flag_is_last_hor_MCU = 1;
				extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
				idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
				idct_col_num_CbCr_ForLast = idct_col_num_Y_block1>>1;//change019
			}
			else{
				flag_is_last_hor_MCU = 0;
				idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
			}
			idct_col_num_Y = idct_col_num_Y_block1;//change014
			idct_row_num_Y = idct_row_num_Y1;
			MCU_hor_partsum_first = 0; //bugfix002
			block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
			block_ver_partsum_count_Y = row_ver_partsum_count_Y;
			IDCT_BilScaling_Routine(0);
			if(flag_is_last_hor_MCU==0){
				//update final output index for next block
#if 0				
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block //vertical is not changed
				//manson: MCU_output_ver_count has been assigned default value 0 already
				//this is needed because if the picture size is very large, MCU_output_ver_count may really has 0 value
				MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
				ver_final_Y_index_block += block_ver_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block //vertical is not changed
				//manson: MCU_output_ver_count has been assigned default value 0 already
				//this is needed because if the picture size is very large, MCU_output_ver_count may really has 0 value
				MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
				if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
					max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
			}
			if(Y_only!=0) //for progressive JPEG component Y
				MCU_block_counter = 0;
		}
		break;
	case JPGD_GRAYSCALE:
		//change014
		decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
		if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
			flag_is_last_hor_MCU = 1;
			extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
			idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
			idct_col_num_CbCr_ForLast = idct_col_num_Y_ForMost-extra_columns;
		}
		else{
			flag_is_last_hor_MCU = 0;
			idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
		}
		idct_col_num_Y = idct_col_num_Y_block1;//change014
		idct_row_num_Y = idct_row_num_Y1;
		MCU_hor_partsum_first = 0; //bugfix002
		block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		block_ver_partsum_count_Y = row_ver_partsum_count_Y;
		IDCT_BilScaling_Routine(0);
		//update final output index for next block
		hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
		//ver_final_Y_index_block //vertical is not changed
		MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
		if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
			max_MCU_output_ver_count_Y = MCU_output_ver_count;
		break;
	}
	//JPEG_EXIT;
}

static void IDCT_Scaling_180(pImagedec_hdl phdl, int Y_only)
{
	int temp_height;
	int extra_columns; //change014
/*	
	//use this debug code to verify 4x4 IDCT
	int debug_IDCT_buf1[64];
	int debug_IDCT_buf2[64];
	int i;

	for(i=0;i<64;i++)
		debug_IDCT_buf1[i] = *(pInputData+i);
	idct(pInputData, temp_IDCT_buf2);
	idct4x1(debug_IDCT_buf1, debug_IDCT_buf2);
	//for(i=0;i<16;i++){ //4x4
	//for(i=0;i<4;i++){ //2x2
	//for(i=0;i<32;i++){ //8x4
	//for(i=0;i<8;i++){ //4x2
	//for(i=0;i<2;i++){ //2x1
	//for(i=0;i<16;i++){ //8x2
	for(i=0;i<4;i++){ //4x1
		if(debug_IDCT_buf2[i] != temp_IDCT_buf2[i])
			debug_counter++;
	}
	debug_counter2++;
*/

	MCU_block_counter++;
	
	switch (phdl->decoder.out_type){
	case JPGD_YH2V2: //4:2:0
		if(MCU_block_counter==5){//libc_printf("MCU_block_counter:%d\n",MCU_block_counter); //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
			hor_final_Cr_index_block -= hor_final_output_counter_Cr;
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==6){//libc_printf("MCU_block_counter:%d\n",MCU_block_counter); //Cb //bugfix002
			//manson: use idct_col_num_CbCr of MCU_block_counter==5
			IDCT_BilScaling_Routine(1);
			hor_final_Cb_index_block -= hor_final_output_counter_Cb;		
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		//	SDBBP();
		}
		else{
		
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 1); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block2 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 1)-extra_columns)>>1;
					jpeg_printf("extra <%d>idct_col_1 <%d>2<%d>\n",extra_columns,idct_col_num_Y_block1,idct_col_num_Y_block2);					
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
				}

				idct_col_num_Y = idct_col_num_Y_block1; //change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;

				//change016; for supporting 4:2:0
				ver_integer_counter_Y1 = row_ver_integer_counter_Y; //change 016
				ver_integer_counter_Y2 = row_ver_integer_counter_Y; //for support 4:2:0, block 1-3 share one counter, block 2-4 share another
				ver_fraction_counter_Y1 = row_ver_fraction_counter_Y; 
				ver_fraction_counter_Y2 = row_ver_fraction_counter_Y; 
				ver_partsum_count_Y1 = row_ver_partsum_count_Y; //change018
				ver_partsum_count_Y2 = row_ver_partsum_count_Y; //change018

				block_ver_integer_counter_Y = ver_integer_counter_Y1; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y1;
				block_ver_partsum_count_Y = ver_partsum_count_Y1;
				
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y1 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y1 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y1 = block_ver_partsum_count_Y;
				hor_final_Y_index_block = hor_final_Y_index_MCU - block_hor_final_output_counter_Y; //horizontal is increased  
			//	libc_printf("type<1>hor_final_Y_index_MCU:%d block_hor_final_output_counter_Y:%d\n",hor_final_Y_index_MCU,block_hor_final_output_counter_Y);
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
				first_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
				break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;

				block_ver_integer_counter_Y = ver_integer_counter_Y2; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y2;
				block_ver_partsum_count_Y = ver_partsum_count_Y2;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y2 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y2 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y2 = block_ver_partsum_count_Y;
				
				hor_final_Y_index_MCU_tophalf = hor_final_Y_index_block - block_hor_final_output_counter_Y; //change014
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU 
				//manson: if the picture size is very large and vertical, sometimes 
				//for some blocks block_ver_final_output_counter_Y can be zero
				//the block3's output will be written to a wrong position
				//to avoid this case, use the non-zero vertical counter of block 1 or 2
				//there is a very special case that the last MCU may totally has no output
				//because the picture size is not integer of MCU size. So we only update 
				//MCU_output_ver_count when the MCU is not the last one.
				//manson: although the last block has less column, it is still possible
				//that the first block doesn't have output while the last block has
				if(block_ver_final_output_counter_Y!=0){ //change016
					ver_final_Y_index_block = ver_final_Y_index_MCU - block_ver_final_output_counter_Y; //vertical is increased  
				}
				else{
					ver_final_Y_index_block = ver_final_Y_index_MCU - first_ver_final_output_counter_Y; //vertical is increased  
				}
				//change018
				if(block_ver_final_output_counter_Y!=0){//change017
					MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_ver_count = first_ver_final_output_counter_Y; //bugfix002
				}
			break;
			case 3: //third Y block
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;

				block_ver_integer_counter_Y = ver_integer_counter_Y1; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y1;
				block_ver_partsum_count_Y = ver_partsum_count_Y1;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y1 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y1 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y1 = block_ver_partsum_count_Y;
				
				hor_final_Y_index_block = hor_final_Y_index_MCU - block_hor_final_output_counter_Y; //horizontal is increased  
				third_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
				//ver_final_Y_index_block vertical is the same as block 3
			break;
			case 4: //fourth Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;
				
				block_ver_integer_counter_Y = ver_integer_counter_Y2; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y2;
				block_ver_partsum_count_Y = ver_partsum_count_Y2;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y2 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y2 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y2 = block_ver_partsum_count_Y;

				//change018 manson: if the picture size is very large, top half or bottom half may have 0 output
				hor_final_Y_index_block = hor_final_Y_index_block - block_hor_final_output_counter_Y; //change018
				if(hor_final_Y_index_block < hor_final_Y_index_MCU_tophalf)
					hor_final_Y_index_MCU = hor_final_Y_index_block;
				else
					hor_final_Y_index_MCU = hor_final_Y_index_MCU_tophalf; //change014
				//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
				//MCU final output vertical address will be updated after one row of MCU processing
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 

				//manson: for the last MCU line, if the picture size is not integer times of MCU size
				//we shall take the top half of the MCU's block_hor_final_output_counter_Y
				//if the picture size is very large, for example 8000x6000, the first
				//16x16 MCU may not have output. So we can not always take the first MCU output counter for the whole MCU row
				if(block_ver_final_output_counter_Y!=0){
					MCU_output_ver_count = MCU_output_ver_count + block_ver_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_ver_count = MCU_output_ver_count + third_ver_final_output_counter_Y;
				}
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH2V1: //4:2:2
		if(MCU_block_counter==3){ //Cr
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			hor_final_Cr_index_block -= hor_final_output_counter_Cr;
#endif
			//ver_final_Cr_index_block //vertical is not changed
		}
		else if(MCU_block_counter==4){ //Cb
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			hor_final_Cb_index_block -= hor_final_output_counter_Cb;
#endif			//ver_final_Cb_index_block //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			//final output address for this block has been updated when the previous block is finished
			//update final output index for next block
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 1); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block2 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 1)-extra_columns)>>1;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
				}
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0; //bugfix002

				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0);
				first_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
				hor_final_Y_index_block = hor_final_Y_index_MCU - block_hor_final_output_counter_Y; //horizontal is increased  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
				break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0; //bugfix002

				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0); //change014
				if(flag_is_last_hor_MCU==0){
					hor_final_Y_index_MCU = hor_final_Y_index_block - block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					if(block_ver_final_output_counter_Y!=0)
						MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
					else
						MCU_output_ver_count = first_ver_final_output_counter_Y;
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH1V2: //vertically decimated by 2, horizontal the same as Y
		//I don't want to spend my time to write a 4x8 IDCT
		//but it is not necessary to optimize since it is not popular anyway
		if(MCU_block_counter==3){ //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			hor_final_Cr_index_block -= hor_final_output_counter_Cr;
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==4){ //Cb //bugfix002
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			hor_final_Cb_index_block -= hor_final_output_counter_Cb;
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
					idct_col_num_CbCr_ForLast = idct_col_num_Y_ForMost-extra_columns;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
				}
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;
				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0);
				hor_final_Y_index_MCU_tophalf = hor_final_Y_index_MCU - block_hor_final_output_counter_Y; //change014
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU 
				ver_final_Y_index_block = ver_final_Y_index_MCU - block_ver_final_output_counter_Y; //vertical is increased  
				//change017
				MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
			break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;
				//block_ver_integer_counter_Y and block_ver_fraction_counter_Y and block_ver_partsum_count_Y are updated in first block
				IDCT_BilScaling_Routine(0);
				if(flag_is_last_hor_MCU==0){
					//change018
					hor_final_Y_index_block = hor_final_Y_index_block - block_hor_final_output_counter_Y; //change018
					if(hor_final_Y_index_block < hor_final_Y_index_MCU_tophalf)
						hor_final_Y_index_MCU = hor_final_Y_index_block;
					else
						hor_final_Y_index_MCU = hor_final_Y_index_MCU_tophalf; //change014
					//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_ver_count = MCU_output_ver_count + block_ver_final_output_counter_Y; //change017
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH4V1: //4:1:1
		if(MCU_block_counter==5){ //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			hor_final_Cr_index_block -= hor_final_output_counter_Cr;
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==6){ //Cb //bugfix002
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			hor_final_Cb_index_block -= hor_final_output_counter_Cb;
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			//final output address for this block has been updated when the previous block is finished
			idct_row_num_Y = idct_row_num_Y1;
			block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
			block_ver_partsum_count_Y = row_ver_partsum_count_Y;
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 2); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > (idct_col_num_Y_ForMost*3)){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 2) - extra_columns;
						idct_col_num_Y_block2 = 0;
						idct_col_num_Y_block3 = 0;
						idct_col_num_Y_block4 = 0;
					}
					else if(extra_columns > (idct_col_num_Y_ForMost<<1)){
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = (idct_col_num_Y_ForMost * 3) - extra_columns;
						idct_col_num_Y_block3 = 0;
						idct_col_num_Y_block4 = 0;
					}
					else if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block3 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block4 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block3 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block4 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 2)-extra_columns)>>2;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block3 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block4 = idct_col_num_Y_ForMost;
				}
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				IDCT_BilScaling_Routine(0);
				hor_final_Y_index_block = hor_final_Y_index_MCU - block_hor_final_output_counter_Y; //horizontal is increased  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
				break;
			case 2: //second Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				IDCT_BilScaling_Routine(0);
				hor_final_Y_index_block = hor_final_Y_index_block - block_hor_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical not change
			break;
			case 3: //third Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block3;//change014
				IDCT_BilScaling_Routine(0);
				hor_final_Y_index_block = hor_final_Y_index_block - block_hor_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical not change
			break;
			case 4: //fourth Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block4;//change014
				IDCT_BilScaling_Routine(0);
				if(flag_is_last_hor_MCU==0){
					hor_final_Y_index_MCU = hor_final_Y_index_block - block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH1V1: //4:4:4
		if(MCU_block_counter==2){ //Cr
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			hor_final_Cr_index_block -= hor_final_output_counter_Cr;
#endif
			//ver_final_Cr_index_block //vertical is not changed
		}
		else if(MCU_block_counter==3){ //Cb
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			hor_final_Cb_index_block -= hor_final_output_counter_Cb;
#endif
			//ver_final_Cb_index_block //vertical is not change 
			MCU_block_counter = 0;
		}
		else if(MCU_block_counter==1){ //Y
			//change014
			decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
			if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
				flag_is_last_hor_MCU = 1;
				extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
				idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
				idct_col_num_CbCr_ForLast = idct_col_num_Y_block1>>1;//change019
			}
			else{
				flag_is_last_hor_MCU = 0;
				idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
			}
			idct_col_num_Y = idct_col_num_Y_block1;//change014
			idct_row_num_Y = idct_row_num_Y1;
			MCU_hor_partsum_first = 0; //bugfix002
			block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
			block_ver_partsum_count_Y = row_ver_partsum_count_Y;
			IDCT_BilScaling_Routine(0);
			if(flag_is_last_hor_MCU==0){
				//update final output index for next block
				hor_final_Y_index_block = hor_final_Y_index_block - block_hor_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block //vertical is not changed
				//manson: MCU_output_ver_count has been assigned default value 0 already
				//this is needed because if the picture size is very large, MCU_output_ver_count may really has 0 value
				MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
			}
			if(Y_only!=0) //for progressive JPEG component Y
				MCU_block_counter = 0;
		}
		break;
	case JPGD_GRAYSCALE:
		//change014
		decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
		if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
			flag_is_last_hor_MCU = 1;
			extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
			idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
			idct_col_num_CbCr_ForLast = idct_col_num_Y_ForMost-extra_columns;
		}
		else{
			flag_is_last_hor_MCU = 0;
			idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
		}
		idct_col_num_Y = idct_col_num_Y_block1;//change014
		idct_row_num_Y = idct_row_num_Y1;
		MCU_hor_partsum_first = 0; //bugfix002
		block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		block_ver_partsum_count_Y = row_ver_partsum_count_Y;
		IDCT_BilScaling_Routine(0);
		//update final output index for next block
		hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
		//ver_final_Y_index_block //vertical is not changed
		MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
		if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
			max_MCU_output_ver_count_Y = MCU_output_ver_count;
		break;
	}
}

static void IDCT_Scaling_90C(pImagedec_hdl phdl, int Y_only)
{
	int temp_height;
	int extra_columns; //change014
/*	
	//use this debug code to verify 4x4 IDCT
	int debug_IDCT_buf1[64];
	int debug_IDCT_buf2[64];
	int i;

	for(i=0;i<64;i++)
		debug_IDCT_buf1[i] = *(pInputData+i);
	idct(pInputData, temp_IDCT_buf2);
	idct4x1(debug_IDCT_buf1, debug_IDCT_buf2);
	//for(i=0;i<16;i++){ //4x4
	//for(i=0;i<4;i++){ //2x2
	//for(i=0;i<32;i++){ //8x4
	//for(i=0;i<8;i++){ //4x2
	//for(i=0;i<2;i++){ //2x1
	//for(i=0;i<16;i++){ //8x2
	for(i=0;i<4;i++){ //4x1
		if(debug_IDCT_buf2[i] != temp_IDCT_buf2[i])
			debug_counter++;
	}
	debug_counter2++;
*/
	//JPEG_ENTRY;
	MCU_block_counter++;
	switch (phdl->decoder.out_type){
	case JPGD_YH2V2: //4:2:0
		if(MCU_block_counter==5){//libc_printf("MCU_block_counter:%d\n",MCU_block_counter); //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block -= ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==6){//libc_printf("MCU_block_counter:%d\n",MCU_block_counter); //Cb //bugfix002
			//manson: use idct_col_num_CbCr of MCU_block_counter==5
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block -= ver_final_output_counter_Cb; //horizontal is increased  
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		//	SDBBP();
		}
		else{
		
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 1); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block2 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 1)-extra_columns)>>1;
					jpeg_printf("extra <%d>idct_col_1 <%d>2<%d>\n",extra_columns,idct_col_num_Y_block1,idct_col_num_Y_block2);					
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
				}

				idct_col_num_Y = idct_col_num_Y_block1; //change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;

				//change016; for supporting 4:2:0
				ver_integer_counter_Y1 = row_ver_integer_counter_Y; //change 016
				ver_integer_counter_Y2 = row_ver_integer_counter_Y; //for support 4:2:0, block 1-3 share one counter, block 2-4 share another
				ver_fraction_counter_Y1 = row_ver_fraction_counter_Y; 
				ver_fraction_counter_Y2 = row_ver_fraction_counter_Y; 
				ver_partsum_count_Y1 = row_ver_partsum_count_Y; //change018
				ver_partsum_count_Y2 = row_ver_partsum_count_Y; //change018

				block_ver_integer_counter_Y = ver_integer_counter_Y1; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y1;
				block_ver_partsum_count_Y = ver_partsum_count_Y1;
				
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y1 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y1 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y1 = block_ver_partsum_count_Y;

#if 0
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
			//	libc_printf("type<1>hor_final_Y_index_MCU:%d block_hor_final_output_counter_Y:%d\n",hor_final_Y_index_MCU,block_hor_final_output_counter_Y);
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
				first_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
#else
				ver_final_Y_index_block = ver_final_Y_index_MCU - block_ver_final_output_counter_Y;
				hor_final_Y_index_block = hor_final_Y_index_MCU;
				first_hor_final_output_counter_Y = block_hor_final_output_counter_Y;
#endif				
				break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;

				block_ver_integer_counter_Y = ver_integer_counter_Y2; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y2;
				block_ver_partsum_count_Y = ver_partsum_count_Y2;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y2 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y2 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y2 = block_ver_partsum_count_Y;

#if 0
				hor_final_Y_index_MCU_tophalf = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change014
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU 
				//manson: if the picture size is very large and vertical, sometimes 
				//for some blocks block_ver_final_output_counter_Y can be zero
				//the block3's output will be written to a wrong position
				//to avoid this case, use the non-zero vertical counter of block 1 or 2
				//there is a very special case that the last MCU may totally has no output
				//because the picture size is not integer of MCU size. So we only update 
				//MCU_output_ver_count when the MCU is not the last one.
				//manson: although the last block has less column, it is still possible
				//that the first block doesn't have output while the last block has
				if(block_ver_final_output_counter_Y!=0){ //change016
					ver_final_Y_index_block = ver_final_Y_index_MCU+block_ver_final_output_counter_Y; //vertical is increased  
				}
				else{
					ver_final_Y_index_block = ver_final_Y_index_MCU+first_ver_final_output_counter_Y; //vertical is increased  
				}
				//change018
				if(block_ver_final_output_counter_Y!=0){//change017
					MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_ver_count = first_ver_final_output_counter_Y; //bugfix002
				}
#else
				ver_final_Y_index_MCU_tophalf = ver_final_Y_index_block - block_ver_final_output_counter_Y; //change014
				ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU 
				//manson: if the picture size is very large and vertical, sometimes 
				//for some blocks block_ver_final_output_counter_Y can be zero
				//the block3's output will be written to a wrong position
				//to avoid this case, use the non-zero vertical counter of block 1 or 2
				//there is a very special case that the last MCU may totally has no output
				//because the picture size is not integer of MCU size. So we only update 
				//MCU_output_ver_count when the MCU is not the last one.
				//manson: although the last block has less column, it is still possible
				//that the first block doesn't have output while the last block has
				if(block_hor_final_output_counter_Y!=0){ //change016
					hor_final_Y_index_block = hor_final_Y_index_MCU + block_hor_final_output_counter_Y; //vertical is increased  
				}
				else{
					hor_final_Y_index_block = hor_final_Y_index_MCU + first_hor_final_output_counter_Y; //vertical is increased  
				}
				//change018
				if(block_hor_final_output_counter_Y!=0){//change017
					MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_hor_count = first_hor_final_output_counter_Y; //bugfix002
				}
#endif				
				break;
			case 3: //third Y block
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;

				block_ver_integer_counter_Y = ver_integer_counter_Y1; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y1;
				block_ver_partsum_count_Y = ver_partsum_count_Y1;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y1 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y1 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y1 = block_ver_partsum_count_Y;

#if 0				
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				third_ver_final_output_counter_Y = block_ver_final_output_counter_Y;
				//ver_final_Y_index_block vertical is the same as block 3
#else
				ver_final_Y_index_block = ver_final_Y_index_MCU  - block_ver_final_output_counter_Y; //horizontal is increased  
				third_hor_final_output_counter_Y = block_hor_final_output_counter_Y;
#endif
				break;
			case 4: //fourth Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;
				
				block_ver_integer_counter_Y = ver_integer_counter_Y2; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = ver_fraction_counter_Y2;
				block_ver_partsum_count_Y = ver_partsum_count_Y2;
				IDCT_BilScaling_Routine(0);
				ver_integer_counter_Y2 = block_ver_integer_counter_Y;
				ver_fraction_counter_Y2 = block_ver_fraction_counter_Y;
				ver_partsum_count_Y2 = block_ver_partsum_count_Y;

#if 0
				//change018 manson: if the picture size is very large, top half or bottom half may have 0 output
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change018
				if(hor_final_Y_index_block > hor_final_Y_index_MCU_tophalf)
					hor_final_Y_index_MCU = hor_final_Y_index_block;
				else
					hor_final_Y_index_MCU = hor_final_Y_index_MCU_tophalf; //change014
				//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
				//MCU final output vertical address will be updated after one row of MCU processing
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 

				//manson: for the last MCU line, if the picture size is not integer times of MCU size
				//we shall take the top half of the MCU's block_hor_final_output_counter_Y
				//if the picture size is very large, for example 8000x6000, the first
				//16x16 MCU may not have output. So we can not always take the first MCU output counter for the whole MCU row
				if(block_ver_final_output_counter_Y!=0){
					MCU_output_ver_count = MCU_output_ver_count + block_ver_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_ver_count = MCU_output_ver_count + third_ver_final_output_counter_Y;
				}
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
				ver_final_Y_index_block = ver_final_Y_index_block - block_ver_final_output_counter_Y; //change018
				if(ver_final_Y_index_block < ver_final_Y_index_MCU_tophalf)
					ver_final_Y_index_MCU = ver_final_Y_index_block;
				else
					ver_final_Y_index_MCU = ver_final_Y_index_MCU_tophalf; //change014
					
				//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
				//MCU final output vertical address will be updated after one row of MCU processing
				ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU  
				hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 

				//manson: for the last MCU line, if the picture size is not integer times of MCU size
				//we shall take the top half of the MCU's block_hor_final_output_counter_Y
				//if the picture size is very large, for example 8000x6000, the first
				//16x16 MCU may not have output. So we can not always take the first MCU output counter for the whole MCU row
				if(block_hor_final_output_counter_Y!=0){
					MCU_output_hor_count += block_hor_final_output_counter_Y; //bugfix002
				}
				else{
					MCU_output_hor_count += third_hor_final_output_counter_Y;
				}
				if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
					max_MCU_output_hor_count_Y = MCU_output_hor_count;
				if(max_MCU_output_hor_count_Y > 256)
					SDBBP();
#endif
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH2V1: //4:2:2
		if(MCU_block_counter==3){ //Cr
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block -= ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block //vertical is not changed
		}
		else if(MCU_block_counter==4){ //Cb
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block -= ver_final_output_counter_Cb; //horizontal is increased  
#endif			
			//ver_final_Cb_index_block //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			//final output address for this block has been updated when the previous block is finished
			//update final output index for next block
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 1); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block2 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 1)-extra_columns)>>1;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
				}
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0; //bugfix002

				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0);
#if 0				
				first_ver_final_output_counter_Y = block_ver_final_output_counter_Y;				
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
#else
				first_hor_final_output_counter_Y = block_hor_final_output_counter_Y;				
				ver_final_Y_index_block -= block_ver_final_output_counter_Y; //horizontal is increased  
				hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
#endif
				break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0; //bugfix002

				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0); //change014
				if(flag_is_last_hor_MCU==0){

#if 0		
					hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					if(block_ver_final_output_counter_Y!=0)
						MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
					else
						MCU_output_ver_count = first_ver_final_output_counter_Y;
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
					ver_final_Y_index_MCU = ver_final_Y_index_block - block_ver_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU  
					hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
					if(block_hor_final_output_counter_Y!=0)
						MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
					else
						MCU_output_hor_count = first_hor_final_output_counter_Y;
					if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
						max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH1V2: //vertically decimated by 2, horizontal the same as Y
		//I don't want to spend my time to write a 4x8 IDCT
		//but it is not necessary to optimize since it is not popular anyway
		if(MCU_block_counter==3){ //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block -= ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==4){ //Cb //bugfix002
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block -= ver_final_output_counter_Cb; //horizontal is increased  
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else{
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
					idct_col_num_CbCr_ForLast = idct_col_num_Y_ForMost-extra_columns;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
				}
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y1;
				MCU_hor_partsum_first = 0;
				block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
				block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
				block_ver_partsum_count_Y = row_ver_partsum_count_Y;
				IDCT_BilScaling_Routine(0);
#if 0				
				hor_final_Y_index_MCU_tophalf = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //change014
				hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU 
				ver_final_Y_index_block = ver_final_Y_index_MCU+block_ver_final_output_counter_Y; //vertical is increased  
				//change017
				MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
				ver_final_Y_index_MCU_tophalf = ver_final_Y_index_MCU-block_ver_final_output_counter_Y; //change014
				ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU 
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //vertical is increased  
				//change017
				MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
				if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
					max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				
			break;
			case 2: //second Y block
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				idct_row_num_Y = idct_row_num_Y2;
				MCU_hor_partsum_first = 8;
				//block_ver_integer_counter_Y and block_ver_fraction_counter_Y and block_ver_partsum_count_Y are updated in first block
				IDCT_BilScaling_Routine(0);
				if(flag_is_last_hor_MCU==0){
					//change018
#if	0				
					hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //change018
					if(hor_final_Y_index_block > hor_final_Y_index_MCU_tophalf)
						hor_final_Y_index_MCU = hor_final_Y_index_block;
					else
						hor_final_Y_index_MCU = hor_final_Y_index_MCU_tophalf; //change014
					//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_ver_count = MCU_output_ver_count + block_ver_final_output_counter_Y; //change017
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
					ver_final_Y_index_block -= block_ver_final_output_counter_Y; //change018
					if(ver_final_Y_index_block < ver_final_Y_index_MCU_tophalf)
						ver_final_Y_index_MCU = ver_final_Y_index_block;
					else
						ver_final_Y_index_MCU = ver_final_Y_index_MCU_tophalf; //change014
					//hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU  
					hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_hor_count += block_hor_final_output_counter_Y; //change017
					if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
						max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH4V1: //4:1:1
		if(MCU_block_counter==5){ //Cr //bugfix002
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block -= ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==6){ //Cb //bugfix002
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block -= ver_final_output_counter_Cb; //horizontal is increased  
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			
			MCU_block_counter = 0;
		}
		else{
			//final output address for this block has been updated when the previous block is finished
			idct_row_num_Y = idct_row_num_Y1;
			block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
			block_ver_partsum_count_Y = row_ver_partsum_count_Y;
			switch(MCU_block_counter){
			case 1:
				//change014
				decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + (idct_col_num_Y_ForMost << 2); //change014
				if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
					flag_is_last_hor_MCU = 1;
					extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
					if(extra_columns > (idct_col_num_Y_ForMost*3)){
						idct_col_num_Y_block1 = (idct_col_num_Y_ForMost << 2) - extra_columns;
						idct_col_num_Y_block2 = 0;
						idct_col_num_Y_block3 = 0;
						idct_col_num_Y_block4 = 0;
					}
					else if(extra_columns > (idct_col_num_Y_ForMost<<1)){
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = (idct_col_num_Y_ForMost * 3) - extra_columns;
						idct_col_num_Y_block3 = 0;
						idct_col_num_Y_block4 = 0;
					}
					else if(extra_columns > idct_col_num_Y_ForMost){
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block3 = (idct_col_num_Y_ForMost << 1) - extra_columns;
						idct_col_num_Y_block4 = 0;
					}
					else{
						idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block3 = idct_col_num_Y_ForMost;
						idct_col_num_Y_block4 = idct_col_num_Y_ForMost - extra_columns;
					}
					idct_col_num_CbCr_ForLast = ((idct_col_num_Y_ForMost << 2)-extra_columns)>>2;
				}
				else{
					flag_is_last_hor_MCU = 0;
					idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block2 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block3 = idct_col_num_Y_ForMost;
					idct_col_num_Y_block4 = idct_col_num_Y_ForMost;
				}
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block1;//change014
				IDCT_BilScaling_Routine(0);
#if 0				
				hor_final_Y_index_block = hor_final_Y_index_MCU+block_hor_final_output_counter_Y; //horizontal is increased  
				ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
#else
				ver_final_Y_index_block = ver_final_Y_index_MCU-block_ver_final_output_counter_Y; //horizontal is increased  
				hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
#endif
				break;
			case 2: //second Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block2;//change014
				IDCT_BilScaling_Routine(0);
#if 0				
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
#else
				ver_final_Y_index_block -= block_ver_final_output_counter_Y; //horizontal is increased  
#endif
				//ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical not change
			break;
			case 3: //third Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block3;//change014
				IDCT_BilScaling_Routine(0);
#if 0				
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
#else
				ver_final_Y_index_block -= block_ver_final_output_counter_Y; 
#endif
				//ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical not change
			break;
			case 4: //fourth Y block
				MCU_hor_partsum_first = 0;
				idct_col_num_Y = idct_col_num_Y_block4;//change014
				IDCT_BilScaling_Routine(0);
				if(flag_is_last_hor_MCU==0){
#if 0					
					hor_final_Y_index_MCU = hor_final_Y_index_block+block_hor_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					hor_final_Y_index_block = hor_final_Y_index_MCU; //horizontal is the same as MCU  
					ver_final_Y_index_block = ver_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
					if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
						max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
					ver_final_Y_index_MCU = ver_final_Y_index_block - block_ver_final_output_counter_Y; //update MCU output horizontal index
					//MCU final output vertical address will be updated after one row of MCU processing
					ver_final_Y_index_block = ver_final_Y_index_MCU; //horizontal is the same as MCU  
					hor_final_Y_index_block = hor_final_Y_index_MCU; //vertical is the same as MCU 
					MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
					if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
						max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
				}
				if(Y_only!=0) //for progressive JPEG component Y
					MCU_block_counter = 0;
				break;
			}
		}
		break;
	case JPGD_YH1V1: //4:4:4
		if(MCU_block_counter==2){ //Cr
			if(flag_is_last_hor_MCU==0)
				idct_col_num_CbCr = idct_col_num_CbCr_ForMost;
			else
				idct_col_num_CbCr = idct_col_num_CbCr_ForLast;
			IDCT_BilScaling_Routine(2);
#if 0			
			hor_final_Cr_index_block = hor_final_Cr_index_block+hor_final_output_counter_Cr; //horizontal is increased  
#else
			ver_final_Cr_index_block -= ver_final_output_counter_Cr; 
#endif
			//ver_final_Cr_index_block = ver_final_Cr_index_block; //vertical is the same as MCU 
		}
		else if(MCU_block_counter==3){ //Cb
			IDCT_BilScaling_Routine(1);
#if 0			
			hor_final_Cb_index_block = hor_final_Cb_index_block+hor_final_output_counter_Cb; //horizontal is increased  
#else
			ver_final_Cb_index_block -= ver_final_output_counter_Cb; //horizontal is increased  
#endif
			//ver_final_Cb_index_block = ver_final_Cb_index_block; //vertical is the same as MCU 
			MCU_block_counter = 0;
		}
		else if(MCU_block_counter==1){ //Y
			//change014
			decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
			if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
				flag_is_last_hor_MCU = 1;
				extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
				idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
				idct_col_num_CbCr_ForLast = idct_col_num_Y_block1>>1;//change019
			}
			else{
				flag_is_last_hor_MCU = 0;
				idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
			}
			idct_col_num_Y = idct_col_num_Y_block1;//change014
			idct_row_num_Y = idct_row_num_Y1;
			MCU_hor_partsum_first = 0; //bugfix002
			block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
			block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
			block_ver_partsum_count_Y = row_ver_partsum_count_Y;
			IDCT_BilScaling_Routine(0);
			if(flag_is_last_hor_MCU==0){
				//update final output index for next block
#if 0				
				hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block //vertical is not changed
				//manson: MCU_output_ver_count has been assigned default value 0 already
				//this is needed because if the picture size is very large, MCU_output_ver_count may really has 0 value
				MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
				if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
					max_MCU_output_ver_count_Y = MCU_output_ver_count;
#else
				ver_final_Y_index_block -= block_ver_final_output_counter_Y; //horizontal is increased  
				//ver_final_Y_index_block //vertical is not changed
				//manson: MCU_output_ver_count has been assigned default value 0 already
				//this is needed because if the picture size is very large, MCU_output_ver_count may really has 0 value
				MCU_output_hor_count = block_hor_final_output_counter_Y; //bugfix002
				if(MCU_output_hor_count > max_MCU_output_hor_count_Y)
					max_MCU_output_hor_count_Y = MCU_output_hor_count;
#endif
			}
			if(Y_only!=0) //for progressive JPEG component Y
				MCU_block_counter = 0;
		}
		break;
	case JPGD_GRAYSCALE:
		//change014
		decoded_picture_width_counter_Y = decoded_picture_width_counter_Y + idct_col_num_Y_ForMost; //change014
		if(decoded_picture_width_counter_Y > decoded_picture_real_width_Y){
			flag_is_last_hor_MCU = 1;
			extra_columns = decoded_picture_width_counter_Y - decoded_picture_real_width_Y;
			idct_col_num_Y_block1 = idct_col_num_Y_ForMost - extra_columns;
			idct_col_num_CbCr_ForLast = idct_col_num_Y_ForMost-extra_columns;
		}
		else{
			flag_is_last_hor_MCU = 0;
			idct_col_num_Y_block1 = idct_col_num_Y_ForMost;
		}
		idct_col_num_Y = idct_col_num_Y_block1;//change014
		idct_row_num_Y = idct_row_num_Y1;
		MCU_hor_partsum_first = 0; //bugfix002
		block_ver_integer_counter_Y = row_ver_integer_counter_Y; //vertical counters will be updated when preparing next row of MCU
		block_ver_fraction_counter_Y = row_ver_fraction_counter_Y;
		block_ver_partsum_count_Y = row_ver_partsum_count_Y;
		IDCT_BilScaling_Routine(0);
		//update final output index for next block
		hor_final_Y_index_block = hor_final_Y_index_block+block_hor_final_output_counter_Y; //horizontal is increased  
		//ver_final_Y_index_block //vertical is not changed
		MCU_output_ver_count = block_ver_final_output_counter_Y; //bugfix002
		if(MCU_output_ver_count > max_MCU_output_ver_count_Y)
			max_MCU_output_ver_count_Y = MCU_output_ver_count;
		break;
	}
	//JPEG_EXIT;
}
void IDCT_Scaling_Cr() 
{
	int extra_columns;

	decoded_picture_width_counter_Cr = decoded_picture_width_counter_Cr + idct_col_num_CbCr_ForMost; //change014
	if(decoded_picture_width_counter_Cr > decoded_picture_real_width_CbCr){
		flag_is_last_hor_MCU = 1;
		extra_columns = decoded_picture_width_counter_Cr - decoded_picture_real_width_CbCr;
		idct_col_num_CbCr_ForLast = idct_col_num_CbCr_ForMost-extra_columns;
	}
	else{
		flag_is_last_hor_MCU = 0;
	}
	//idct_row_num_CbCr may have been chaned in PrepareForNextRow for avoiding pointer overflow
	IDCT_BilScaling_Routine(2);
	switch(g_imagedec_angle[g_imagedec_hdl[0].id])
	{
			case ANG_90_A:
				ver_final_Cr_index_block += ver_final_output_counter_Cr; //horizontal is increased  								
				break;
			case ANG_180:
				hor_final_Cr_index_block -= hor_final_output_counter_Cr; //horizontal is increased  								
				break;
			case ANG_90_C:
				ver_final_Cr_index_block -= ver_final_output_counter_Cr; //horizontal is increased  	
				break;
			default:
				hor_final_Cr_index_block += hor_final_output_counter_Cr; //horizontal is increased  				
				break;
	}
}

void IDCT_Scaling_Cb() 
{
	int extra_columns;

	decoded_picture_width_counter_Cb = decoded_picture_width_counter_Cb + idct_col_num_CbCr_ForMost; //change014
	if(decoded_picture_width_counter_Cb > decoded_picture_real_width_CbCr){
		flag_is_last_hor_MCU = 1;
		extra_columns = decoded_picture_width_counter_Cb - decoded_picture_real_width_CbCr;
		idct_col_num_CbCr_ForLast = idct_col_num_CbCr_ForMost-extra_columns;
	}
	else{
		flag_is_last_hor_MCU = 0;
	}
	//idct_row_num_Cb may have been chaned in PrepareForNextRow for avoiding pointer overflow
	IDCT_BilScaling_Routine(1);
	switch(g_imagedec_angle[g_imagedec_hdl[0].id])
	{
			case ANG_90_A:
				ver_final_Cb_index_block += ver_final_output_counter_Cb;
				break;
			case ANG_180:
				hor_final_Cb_index_block -= hor_final_output_counter_Cb;
				break;
			case ANG_90_C:
				ver_final_Cb_index_block -= ver_final_output_counter_Cb;
				break;
			default:	
				hor_final_Cb_index_block += hor_final_output_counter_Cb; //horizontal is increased  
				break;
	}
}
extern int cur_decoding_mcu_row;

//manson: for supporting progressive JPEG, I split some functions into three pieces for each components
void PrepareForNextRow(int which_component)
//update some parameters and get ready to process next row of input data
{
	int i;
	int hor_counter;
	int ver_counter;
	uchar *pFinalOutputY;
	uchar *pFinalOutputCb;
	uchar *pFinalOutputCr;
	int ver_output_num2;
	int extra_lines;
//which_component ==0 means all, ==1 means Y, ==2 means Cb, ==3 means Cr

	if((which_component==0)||(which_component==1)){
		if(1 == m_trans_hor_no_scale)
		{
			hor_integer_counter_Y1 = 1; 
			hor_integer_counter_Y2 = 1; 
			hor_fraction_counter_Y1 = 0;//hor_scale_ratio_Y_fraction;//0; change015
			hor_fraction_counter_Y2 = 0;//hor_scale_ratio_Y_fraction;//0; change015
		}
		else
		{
			hor_integer_counter_Y1 = 0; 
			hor_integer_counter_Y2 = 0; 
			hor_fraction_counter_Y1 = hor_scale_ratio_Y_fraction;//0; change015
			hor_fraction_counter_Y2 = hor_scale_ratio_Y_fraction;//0; change015		
		}
		block_first_ver_accumulator_Y1 = 0; 
		block_first_ver_accumulator_Y2 = 0; 
		switch(g_imagedec_angle[g_imagedec_hdl[0].id])
		{	
			case ANG_90_A:
				hor_final_Y_index_MCU -= max_MCU_output_hor_count_Y; //change018
				hor_final_Y_index_block = hor_final_Y_index_MCU;
				ver_final_Y_index_MCU = ver_start_line; 
				ver_final_Y_index_block = ver_start_line;				
				break;
			case ANG_180:
				ver_final_Y_index_MCU -= max_MCU_output_ver_count_Y; //change018
				ver_final_Y_index_block = ver_final_Y_index_MCU;
				hor_final_Y_index_MCU = hor_start_line; 
				hor_final_Y_index_block = hor_start_line;
				break;		
			case ANG_90_C:
				//libc_printf("hor Mcu %d\n",hor_final_Y_index_MCU);
				//if(562 == hor_final_Y_index_MCU)
				//	SDBBP();
				hor_final_Y_index_MCU += max_MCU_output_hor_count_Y; //change018
				hor_final_Y_index_block = hor_final_Y_index_MCU;
				ver_final_Y_index_MCU = ver_start_line; 
				ver_final_Y_index_block = ver_start_line;				
				break;
			default:
				ver_final_Y_index_MCU += max_MCU_output_ver_count_Y; //change018
				ver_final_Y_index_block = ver_final_Y_index_MCU;
				hor_final_Y_index_MCU = hor_start_line; 
				hor_final_Y_index_block = hor_start_line;
				break;
		}
		//if scaling factor is too large, we can not take output counters of the last MCU as output counters of the row
		//because the last MCU may not have output at all
		row_ver_integer_counter_Y = next_row_ver_integer_counter_Y; 
		row_ver_fraction_counter_Y = next_row_ver_fraction_counter_Y;
		row_ver_partsum_count_Y = next_row_ver_partsum_count_Y; //update vertical partsum counter;
		//clear horizontal partsum accumulators
		if(hor_partsum_count_Y1!=0){
			for(i=0;i<16;i++){
				hor_partsum_Y[i] = 0;
			}
			hor_partsum_count_Y1 = 0;
			hor_partsum_count_Y2 = 0;
		}
		decoded_picture_height_counter_Y += idct_row_num_Y * MCU_ver_Y_block_num;
		if(decoded_picture_height_counter_Y > decoded_picture_real_height_Y)
		{
			extra_lines = decoded_picture_height_counter_Y - decoded_picture_real_height_Y; //+1 cover round-off
			if(extra_lines<idct_row_num_Y){
				if(MCU_ver_Y_block_num==1)
					idct_row_num_Y1 = idct_row_num_Y - extra_lines;
				else
					idct_row_num_Y2 = idct_row_num_Y - extra_lines;
			}
			else{ //this is only possible when MCU_ver_Y_block_num == 2
				idct_row_num_Y1 = (idct_row_num_Y << 1) - extra_lines;
				idct_row_num_Y2 = 0;
			}
		}
		decoded_picture_width_counter_Y = 0;  //change014
		max_MCU_output_ver_count_Y = 0; //change018
		MCU_output_ver_count = 0;
		
		max_MCU_output_hor_count_Y = 0; //change018
		MCU_output_hor_count = 0;
	}

	if((which_component==0)||(which_component==2)){
		if(1 == m_trans_hor_no_scale)
		{
			hor_integer_counter_Cb = 1; 
			hor_fraction_counter_Cb = 0;//hor_scale_ratio_CbCr_fraction;//0; change015
		}
		else
		{
			hor_integer_counter_Cb = 0; 
			hor_fraction_counter_Cb = hor_scale_ratio_CbCr_fraction;//0; change015
		}
		block_first_ver_accumulator_Cb = 0; 
		
		switch(g_imagedec_angle[g_imagedec_hdl[0].id])
		{	
			case ANG_90_A:
				ver_final_Cb_index_block = ver_start_line>>1;
				hor_final_Cb_index_block -= row_hor_final_output_counter_Cb;				
				break;
			case ANG_180:
				ver_final_Cb_index_block -= row_ver_final_output_counter_Cb;
				hor_final_Cb_index_block = hor_start_line>>1;
				break;		
			case ANG_90_C:
				ver_final_Cb_index_block = ver_start_line>>1;	
				hor_final_Cb_index_block += row_hor_final_output_counter_Cb;	
				break;
			default:
				ver_final_Cb_index_block += row_ver_final_output_counter_Cb;
				hor_final_Cb_index_block = hor_start_line>>1;
				break;
		}
		
		row_ver_integer_counter_Cb = next_row_ver_integer_counter_Cb; 
		row_ver_fraction_counter_Cb = next_row_ver_fraction_counter_Cb;
		row_ver_partsum_count_Cb = next_row_ver_partsum_count_Cb; 
		if(hor_partsum_count_Cb!=0){
			for(i=0;i<16;i++){
				hor_partsum_Cb[i] = 0;
			}
			hor_partsum_count_Cb = 0;
		}
		decoded_picture_height_counter_Cb = decoded_picture_height_counter_Cb+idct_row_num_CbCr;
		if(decoded_picture_height_counter_Cb > decoded_picture_real_height_CbCr)
		//if(decoded_picture_height_counter_Cb+idct_row_num_CbCr*7 > decoded_picture_real_height_CbCr)
		{
			//frank modify 2003.09.11, only for debugging the green line when rotating some JPEGs.
			//idct_row_num_CbCr = 0;
			//jpeg_printf("==============ver_final_Cb_index_block = %d\n", ver_final_Cb_index_block);

			idct_row_num_CbCr = idct_row_num_CbCr - (decoded_picture_height_counter_Cb - decoded_picture_real_height_CbCr); //+1 cover round-off
			if(idct_row_num_CbCr<0)
				idct_row_num_CbCr = 0;
			if((0 == idct_row_num_CbCr) && (1 == idct_row_num_Y1))
			{
				idct_row_num_CbCr = 1;
			}			
		}
		decoded_picture_width_counter_Cb = 0;
	}

	if((which_component==0)||(which_component==3)){
		if(1 == m_trans_hor_no_scale)
		{
			hor_integer_counter_Cr = 1; 
			hor_fraction_counter_Cr = 0;//hor_scale_ratio_CbCr_fraction;//0; change015
		}
		else
		{
			hor_integer_counter_Cr = 0; 
			hor_fraction_counter_Cr = hor_scale_ratio_CbCr_fraction;//0; change015
		}
		
		block_first_ver_accumulator_Cr = 0; 
		switch(g_imagedec_angle[g_imagedec_hdl[0].id])
		{	
			case ANG_90_A:
				ver_final_Cr_index_block = ver_start_line>>1;
				hor_final_Cr_index_block -= row_hor_final_output_counter_Cr;			
				break;
			case ANG_180:
				ver_final_Cr_index_block -= row_ver_final_output_counter_Cr;
				hor_final_Cr_index_block = hor_start_line>>1;
				break;		
			case ANG_90_C:
				ver_final_Cr_index_block = ver_start_line>>1;
				hor_final_Cr_index_block += row_hor_final_output_counter_Cr;
				break;
			default:
				ver_final_Cr_index_block += row_ver_final_output_counter_Cr;
				hor_final_Cr_index_block = hor_start_line>>1;
				break;
		}
		row_ver_integer_counter_Cr = next_row_ver_integer_counter_Cr; 
		row_ver_fraction_counter_Cr = next_row_ver_fraction_counter_Cr;
		row_ver_partsum_count_Cr = next_row_ver_partsum_count_Cr;
		if(hor_partsum_count_Cr!=0){
			for(i=0;i<16;i++){
				hor_partsum_Cr[i] = 0;
			}
			hor_partsum_count_Cr = 0;
		}
		decoded_picture_height_counter_Cr = decoded_picture_height_counter_Cr+idct_row_num_CbCr;
		if(decoded_picture_height_counter_Cr > decoded_picture_real_height_CbCr){
			idct_row_num_CbCr = idct_row_num_CbCr - (decoded_picture_height_counter_Cr - decoded_picture_real_height_CbCr); //+1 cover round-off
			if(idct_row_num_CbCr<0)
				idct_row_num_CbCr = 0;
			if((0 == idct_row_num_CbCr) && (1 == idct_row_num_Y1))
			{
				idct_row_num_CbCr = 1;
			}
		}
		decoded_picture_width_counter_Cr = 0;
	}
}

#if 0
//frank add
void store_margins()
{
	current_ver_start_line = ver_start_line;
	current_ver_stop_line = ver_stop_line;
	current_hor_start_line = hor_start_line;
	current_hor_stop_line = hor_stop_line;
}
//frank add end;

void ScalingUp()
//scaling up from picture size <= 736x540. including added black lines
{
	int hor_fraction_counter;
	int ver_fraction_counter;
	int hor_scale_factor;
	int ver_scale_factor;
	int hor_counter;
	int ver_counter;
	int curn_output_hor_index;
	int *pLine_buffer_new;
	int *pLine_buffer_old;
	int *pSwap_buffer;
	int *pCurn_hor_output;
	uchar *pCurn_ver_output;
	uchar *ver_output_row;
	uchar *pOriginal_line;
	int fraction;
	int temp_result;


	
	if(flag_need_scaling_up==0)
		return;
	if(decoded_picture_width<720){
		hor_scale_factor = decoded_picture_width * 32768 / 720;
		hor_scale_factor = hor_scale_factor + 1; //avoid overflow
	}
	else{
		hor_scale_factor = 32768;
	}
	if(decoded_picture_height < TV_ver_lines){
		ver_scale_factor = decoded_picture_height * 32768 / TV_ver_lines;
		ver_scale_factor = ver_scale_factor + 1;
	}
	else{
		ver_scale_factor = 32768;
		decoded_picture_height = TV_ver_lines;
	}
	pLine_buffer_old = line_buffer_1;
	pLine_buffer_new = line_buffer_2;
	//start from the last row, from bottom up
	//process Y
	pOriginal_line = final_output_buf_Y + (decoded_picture_height-1) * Final_Output_Buf_Stride_Y;
	ver_output_row = final_output_buf_Y + (TV_ver_lines-1) * Final_Output_Buf_Stride_Y;
	pCurn_ver_output = ver_output_row;
	ver_fraction_counter = 0;
	for(hor_counter=0;hor_counter<720;hor_counter++)
		*(pLine_buffer_old+hor_counter) = 0;
	for(ver_counter=0;ver_counter<decoded_picture_height;ver_counter++){
		hor_fraction_counter = 0;
		pCurn_hor_output = pLine_buffer_new;
		for(hor_counter=1;hor_counter<decoded_picture_width;hor_counter++){
			//horizotal interpolation
			while(hor_counter >= (hor_fraction_counter >> 15)){
				fraction = hor_fraction_counter & 0x7fff;
				temp_result = (int)*(pOriginal_line+hor_counter-1) * (32768 - fraction) + (int)*(pOriginal_line+hor_counter) * fraction;
				*pCurn_hor_output = temp_result >> 8;
				pCurn_hor_output ++;
				hor_fraction_counter = hor_fraction_counter + hor_scale_factor;
			}
		}
		//vertical interpolation
		while(ver_counter >= (ver_fraction_counter >> 15)){
			fraction = ver_fraction_counter & 0x7fff;
			for(hor_counter=1;hor_counter<720;hor_counter++){
				temp_result = (int)*(pLine_buffer_old+hor_counter) * (32768 - fraction) + (int)*(pLine_buffer_new+hor_counter) * fraction;
				*pCurn_ver_output = (uchar)(temp_result >> 22); //22 = 15 + 7
				pCurn_ver_output++;
			}
			ver_fraction_counter = ver_fraction_counter + ver_scale_factor;
			ver_output_row = ver_output_row - Final_Output_Buf_Stride_Y;
			pCurn_ver_output = ver_output_row;
		}
		//swap old new buffer
		pSwap_buffer = pLine_buffer_old;
		pLine_buffer_old = pLine_buffer_new;
		pLine_buffer_new = pSwap_buffer;
		pOriginal_line = pOriginal_line - Final_Output_Buf_Stride_Y;
	}
	//process Cb
//john	pOriginal_line = final_output_buf_Cb + ((decoded_picture_height>>1)-1) * Final_Output_Buf_Stride_CbCr;
//john	ver_output_row = final_output_buf_Cb + ((TV_ver_lines>>1)-1) * Final_Output_Buf_Stride_CbCr;
	pCurn_ver_output = ver_output_row;
	ver_fraction_counter = 0;
	for(hor_counter=0;hor_counter<360;hor_counter++)
		*(pLine_buffer_old+hor_counter) = 128<<7;
	for(ver_counter=0;ver_counter<(decoded_picture_height>>1);ver_counter++){
		hor_fraction_counter = 0;
		pCurn_hor_output = pLine_buffer_new;
		for(hor_counter=1;hor_counter<(decoded_picture_width>>1);hor_counter++){
			//horizotal interpolation
			while(hor_counter >= (hor_fraction_counter >> 15)){
				fraction = hor_fraction_counter & 0x7fff;
				temp_result = (int)*(pOriginal_line+hor_counter-1) * (32768 - fraction) + (int)*(pOriginal_line+hor_counter) * fraction;
				*pCurn_hor_output = temp_result >> 8;
				pCurn_hor_output ++;
				hor_fraction_counter = hor_fraction_counter + hor_scale_factor;
			}
		}
		//vertical interpolation
		while(ver_counter >= (ver_fraction_counter >> 15)){
			fraction = ver_fraction_counter & 0x7fff;
			for(hor_counter=1;hor_counter<360;hor_counter++){
				temp_result = (int)*(pLine_buffer_old+hor_counter) * (32768 - fraction) + (int)*(pLine_buffer_new+hor_counter) * fraction;
				*pCurn_ver_output = (uchar)(temp_result >> 22); //22 = 15 + 7
				pCurn_ver_output++;
			}
			ver_fraction_counter = ver_fraction_counter + ver_scale_factor;
			ver_output_row = ver_output_row - Final_Output_Buf_Stride_CbCr;
			pCurn_ver_output = ver_output_row;
		}
		//swap old new buffer
		pSwap_buffer = pLine_buffer_old;
		pLine_buffer_old = pLine_buffer_new;
		pLine_buffer_new = pSwap_buffer;
		pOriginal_line = pOriginal_line - Final_Output_Buf_Stride_CbCr;
	}
	//process CR
//john	pOriginal_line = final_output_buf_Cr + ((decoded_picture_height>>1)-1) * Final_Output_Buf_Stride_CbCr;
//john	ver_output_row = final_output_buf_Cr + ((TV_ver_lines>>1)-1) * Final_Output_Buf_Stride_CbCr;
	pCurn_ver_output = ver_output_row;
	ver_fraction_counter = 0;
	for(hor_counter=0;hor_counter<360;hor_counter++)
		*(pLine_buffer_old+hor_counter) = 128<<7;
	for(ver_counter=0;ver_counter<(decoded_picture_height>>1);ver_counter++){
		hor_fraction_counter = 0;
		pCurn_hor_output = pLine_buffer_new;
		for(hor_counter=1;hor_counter<(decoded_picture_width>>1);hor_counter++){
			//horizotal interpolation
			while(hor_counter >= (hor_fraction_counter >> 15)){
				fraction = hor_fraction_counter & 0x7fff;
				temp_result = (int)*(pOriginal_line+hor_counter-1) * (32768 - fraction) + (int)*(pOriginal_line+hor_counter) * fraction;
				*pCurn_hor_output = temp_result >> 8;
				pCurn_hor_output ++;
				hor_fraction_counter = hor_fraction_counter + hor_scale_factor;
			}
		}
		//vertical interpolation
		while(ver_counter >= (ver_fraction_counter >> 15)){
			fraction = ver_fraction_counter & 0x7fff;
			for(hor_counter=1;hor_counter<360;hor_counter++){
				temp_result = (int)*(pLine_buffer_old+hor_counter) * (32768 - fraction) + (int)*(pLine_buffer_new+hor_counter) * fraction;
				*pCurn_ver_output = (uchar)(temp_result >> 22); //22 = 15 + 7
				pCurn_ver_output++;
			}
			ver_fraction_counter = ver_fraction_counter + ver_scale_factor;
			ver_output_row = ver_output_row - Final_Output_Buf_Stride_CbCr;
			pCurn_ver_output = ver_output_row;
		}
		//swap old new buffer
		pSwap_buffer = pLine_buffer_old;
		pLine_buffer_old = pLine_buffer_new;
		pLine_buffer_new = pSwap_buffer;
		pOriginal_line = pOriginal_line - Final_Output_Buf_Stride_CbCr;
	}
	//update start stop lines for rotation
	if(ver_scale_factor < 32768){
		ver_start_line = ver_start_line * 32768 / ver_scale_factor;
		ver_stop_line = ver_stop_line * 32768 / ver_scale_factor;
	}
	if(hor_scale_factor < 32768){
		hor_start_line = hor_start_line * 32768 / hor_scale_factor;
		hor_stop_line = hor_stop_line * 32768 / hor_scale_factor;
	}

}

void MaskInvalidLines()
{
	int ver_counter;
	int hor_counter;
	int black_row_index;

	//due to some unknown reason the picture boundary may still have problem in some case
	//to avoid this problem mask a few more lines
	if(ver_start_line!=0) //change009
		ver_start_line = (ver_start_line+2) & 0xfffe;
	if(ver_stop_line!=TV_ver_lines) 
		ver_stop_line = (ver_stop_line-2) & 0xfffe;
	if(hor_start_line!=0) 
		hor_start_line = (hor_start_line+2) & 0xfffe;
	if(hor_stop_line!=720) 
		hor_stop_line = (hor_stop_line-2) & 0xfffe;

	//write the horizontal black lines to the frame buffer
	//Y black line
}
#endif

void Init_Scaling_factors(pImagedec_hdl phdl)
{
	int org_height = 0;
	int org_width = 0;
	int integer_scaled_height = 0; //used for computing scale factors
	int integer_scaled_width = 0; //used for computing scale factors

	int ver_counter;
	int hor_counter;
	int width_x3; //width multiply by 3
	int height_x4; //height multiply by 4
	uchar *pOutput_buf_Y;
	uchar *pOutput_buf_Cb;
	uchar *pOutput_buf_Cr;
	uchar *pOutput_buf_Y_row;
	uchar *pOutput_buf_Cb_row;
	uchar *pOutput_buf_Cr_row;
	int black_line_num;
	int black_row_index;
	int blank_x = 0;
	int blank_y = 0;
	int dst_w = phdl->main.setting.dis.uWidth;
	int dst_h = phdl->main.setting.dis.uHeight;
	int dst_x = phdl->main.setting.dis.uStartX;
	int dst_y = phdl->main.setting.dis.uStartY;
	int bk_w = dst_w;
	int bk_h = dst_h;
	int tmp = 0;
	int i = 0;
	
	if((ANG_90_A == g_imagedec_angle[phdl->id]) || (ANG_90_C == g_imagedec_angle[phdl->id]))
	{
		org_width = phdl->decoder.image_y_size;
		org_height = phdl->decoder.image_x_size;
	}
	else
	{
		org_width = phdl->decoder.image_x_size;
		org_height = phdl->decoder.image_y_size;
	}

	width_x3 = org_width*3;
	height_x4 = (org_height<<2);//height*4

	if(g_imagedec_osd_ins.on)
	{
		final_output_buf_Y = (uchar *)g_imagedec_osd_ins.pic.y_buf;
		final_output_buf_C = (uchar *)g_imagedec_osd_ins.pic.c_buf;
	}
	else
	{
		final_output_buf_Y = (uchar *)phdl->imageout.frm_y_addr;
		final_output_buf_C = (uchar *)phdl->imageout.frm_c_addr;
	}
	
#if 0
	rotated_output_buf_Y = pFBYBuf;
	rotated_output_buf_Cb = pFBCbBuf; 
	rotated_output_buf_Cr = pFBCrBuf;
#endif	

	//manson: initialize some variables defined by me

	//line_buffer_1 = (int*)imagedec_malloc(760<<2);
	//line_buffer_2 = (int*)imagedec_malloc(760<<2);
	hor_partsum_Y = (int16*)imagedec_malloc(16<<1);
	hor_partsum_Cb = (int16*)imagedec_malloc(16<<1);
	hor_partsum_Cr = (int16*)imagedec_malloc(16<<1);
	hor_bilinear_Y1 = (int16*)imagedec_malloc(16<<1);
	hor_bilinear_Cb1 = (int16*)imagedec_malloc(16<<1);
	hor_bilinear_Cr1 = (int16*)imagedec_malloc(16<<1);

	if(g_imagedec_m33_combine_hw_sw_flag)
	{
		ver_partsum_Y = (int*)imagedec_malloc(JPEG_HW_MAX_SCALED_WIDTH_EXT<<2);
		ver_partsum_Cb = (int*)imagedec_malloc(JPEG_HW_MAX_SCALED_WIDTH_EXT<<2);
		ver_partsum_Cr = (int*)imagedec_malloc(JPEG_HW_MAX_SCALED_WIDTH_EXT<<2);
		ver_bilinear_Y1 = (int*)imagedec_malloc(JPEG_HW_MAX_SCALED_WIDTH_EXT<<2);
		ver_bilinear_Cb1 = (int*)imagedec_malloc(JPEG_HW_MAX_SCALED_WIDTH_EXT<<2);
		ver_bilinear_Cr1 = (int*)imagedec_malloc(JPEG_HW_MAX_SCALED_WIDTH_EXT<<2);
	}
	else
	{
		if(g_imagedec_osd_ins.on)
		{
			ver_partsum_Y = (int*)imagedec_malloc(MAX_WIDTH_BY_OSD<<2);
			ver_partsum_Cb = (int*)imagedec_malloc(MAX_WIDTH_BY_OSD<<1);
			ver_partsum_Cr = (int*)imagedec_malloc(MAX_WIDTH_BY_OSD<<1);
			ver_bilinear_Y1 = (int*)imagedec_malloc(MAX_WIDTH_BY_OSD<<2);
			ver_bilinear_Cb1 = (int*)imagedec_malloc(MAX_WIDTH_BY_OSD<<1);
			ver_bilinear_Cr1 = (int*)imagedec_malloc(MAX_WIDTH_BY_OSD<<1);
		}
		else
		{
			ver_partsum_Y = (int*)imagedec_malloc(760<<2);
			ver_partsum_Cb = (int*)imagedec_malloc(370<<2);
			ver_partsum_Cr = (int*)imagedec_malloc(370<<2);
			ver_bilinear_Y1 = (int*)imagedec_malloc(760<<2);
			ver_bilinear_Cb1 = (int*)imagedec_malloc(370<<2);
			ver_bilinear_Cr1 = (int*)imagedec_malloc(370<<2);
		}
	}

	temp_IDCT_buf = (int*)imagedec_malloc(64<<2);
	temp_IDCT_buf2 = (int*)imagedec_malloc(128<<2);

//	MEMSET(hor_partsum_Y,0,16*2); //part sum for horizontal decimation
//	MEMSET(hor_partsum_Cb,0,16*2);
//	MEMSET(hor_partsum_Cr,0,16*2);
//	MEMSET(hor_bilinear_Y1,0,16*2); //bilinear input sample
//	MEMSET(hor_bilinear_Cb1,0,16*2);
//	MEMSET(hor_bilinear_Cr1,0,16*2);
//	MEMSET(ver_partsum_Y,0,730*4); //part sum for vertital decimation, 32 bits
//	MEMSET(ver_partsum_Cb,0,370*4);
//	MEMSET(ver_partsum_Cr,0,370*4);
//	MEMSET(ver_bilinear_Y1,0,730*4); //bilinear input sample
//	MEMSET(ver_bilinear_Cb1,0,370*4);
//	MEMSET(ver_bilinear_Cr1,0,370*4);
	hor_partsum_count_Y1 = 0; //bugfix002
	hor_partsum_count_Y2 = 0; //bugfix002
	hor_partsum_count_Cb = 0;
	hor_partsum_count_Cr = 0;
	row_ver_partsum_count_Y = 0;
	row_ver_partsum_count_Cb = 0;
	row_ver_partsum_count_Cr = 0;
	ver_final_Cb_index = 0;
	ver_final_Cb_index = 0;
	ver_final_Cr_index = 0;
	ver_final_Cr_index = 0;
	MCU_block_counter = 0;
	block_hor_final_output_counter_Y = 0; //counter for horizontal final output
	hor_final_output_counter_Cb = 0; //counter for horizontal final output
	hor_final_output_counter_Cr = 0; //counter for horizontal final output

	ver_final_output_counter_Cb = 0; 
	ver_final_output_counter_Cr = 0; 
	
	block_first_ver_accumulator_Y1 = 0; //index of the first vertical accumulator for the current block
	block_first_ver_accumulator_Y2 = 0; //index of the first vertical accumulator for the current block
	block_first_ver_accumulator_Cb = 0; //index of the first vertical accumulator for the current block
	block_first_ver_accumulator_Cr = 0; //index of the first vertical accumulator for the current block


//	libc_printf("final_output_buf_Y: 0x%x; final_output_buf_C: 0x%x\n", final_output_buf_Y, final_output_buf_C);

	/*align w:h = 4:3*/
	if(width_x3 > height_x4)
	{
		decoded_picture_width = org_width;
		decoded_picture_height = width_x3>>2;
		blank_x = 0;
		blank_y = decoded_picture_height - org_height;
	}
	else if(width_x3 < height_x4)
	{
		decoded_picture_width = height_x4/3;
		decoded_picture_height = org_height;
		blank_x = decoded_picture_width - org_width;
		blank_y = 0;
	}
	else
	{
		decoded_picture_width = org_width;
		decoded_picture_height = org_height;
	}
	/*FULL SRN dst width and pos redefine*/
	if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		dst_x = dst_y = 0;
		if((decoded_picture_width <= bk_w) || (decoded_picture_height <= bk_h))
		{
			dst_w = decoded_picture_width;
			dst_h = decoded_picture_height;

			if(!g_imagedec_osd_ins.on)
			{
				dst_x = (bk_w - dst_w)>>1;
				dst_y = (bk_h - dst_h)>>1;
			}
		}
		
		if(g_imagedec_osd_ins.on)
		{
			blank_x = blank_y = 0;
			dst_w = decoded_picture_width;
			dst_h = decoded_picture_height;
			dst_x = 0;
			dst_y = 0;			
			g_jpeg_dst_stride = g_imagedec_osd_ins.pic.stride>>4;
		}
		else
			g_jpeg_dst_stride = TV_HOR_COL_NUM>>4;
		
	}
	else if (IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)//Thumbnail mode
	{
		bk_w = phdl->imageout.logo_info.width;
		bk_h = phdl->imageout.logo_info.height;
		g_jpeg_dst_stride = phdl->imageout.logo_info.stride;
		dst_w = (dst_w * bk_w) / TV_HOR_COL_NUM;
		dst_h = (dst_h * bk_h) /TV_VER_LINE_NUM;
		dst_x = (dst_x * bk_w)/TV_HOR_COL_NUM;
		dst_y = (dst_y * bk_h)/TV_VER_LINE_NUM;
		if((decoded_picture_width < dst_w) && (decoded_picture_height < dst_h))
		{
			dst_x += (dst_w - decoded_picture_width)>>1;		
			dst_y += (dst_h - decoded_picture_height)>>1;
			dst_w = decoded_picture_width;
			dst_h = decoded_picture_height;
		}
		jpeg_printf("Thumb rect w <%d>h<%d>x <%d>y<%d>\n",dst_w,dst_h,dst_x,dst_y);
	}
	
	phdl->imageout.image_output_stride = TV_HOR_COL_NUM;
	
	/*blank col and row cnt and update it into dst position*/
	if(blank_x)
	{
		blank_x = (blank_x*dst_w)/decoded_picture_width;
		jpeg_printf("blank x <%d>\n",blank_x);
		dst_x += blank_x>>1;
	}
	if(blank_y)
	{
		blank_y = (blank_y*dst_h)/decoded_picture_height;
		jpeg_printf("blank y <%d>\n",blank_y);
		dst_y += blank_y>>1;
	}

	if(g_imagedec_m33_combine_hw_sw_flag)
	{
		if(dst_x)
			dst_w -= dst_x<<1;

		if(dst_y)
			dst_h -= dst_y<<1;
		
		if((ANG_90_A == g_imagedec_angle[phdl->id]) || (ANG_90_C == g_imagedec_angle[phdl->id]))
		{
			decoded_picture_width = phdl->imageout.hw_acce_rect.uHeight;
			decoded_picture_height = phdl->imageout.hw_acce_rect.uWidth;			
		}
		else
		{
			decoded_picture_width = phdl->imageout.hw_acce_rect.uWidth;
			decoded_picture_height = phdl->imageout.hw_acce_rect.uHeight;			
		}
	}
	
	/*position*/
	phdl->imageout.sw_scaled_rect.uStartX = dst_x;
	phdl->imageout.sw_scaled_rect.uStartY = dst_y;
	phdl->imageout.sw_scaled_rect.uWidth = dst_w;
	phdl->imageout.sw_scaled_rect.uHeight = dst_h;

	if(g_imagedec_m33_combine_hw_sw_flag)
	{
		phdl->imageout.image_out_x = 0;
		phdl->imageout.image_out_y = 0;
		phdl->imageout.image_out_w = 720;
		phdl->imageout.image_out_h = 576;	
	}
	else
	{
		if((decoded_picture_width <= bk_w) || (decoded_picture_height <= bk_h))
		{
			phdl->imageout.image_out_x = 0;
			phdl->imageout.image_out_y = 0;
			phdl->imageout.image_out_w = bk_w;
			phdl->imageout.image_out_h = bk_h;				
		}
		else
		{
			phdl->imageout.image_out_x = dst_x;
			phdl->imageout.image_out_y = dst_y;
			phdl->imageout.image_out_w = dst_w;
			phdl->imageout.image_out_h = dst_h;	
		}
	}
	
	hor_start_line = dst_x;
	ver_start_line = dst_y;
	//hor_stop_line  = dst_x + dst_w;
	//ver_stop_line = dst_y + dst_h - blank_y;
	IDCT_BilScaling_Routine = IDCT_BilinearScaling_ORI;
	IDCT_Scaling_Routine = IDCT_Scaling_ORI;
	//if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	//{
	if(g_imagedec_m33_combine_hw_sw_flag)
	{
		switch(g_imagedec_angle[phdl->id])
		{
			case ANG_90_A:
				hor_start_line += dst_w - 1;
				IDCT_BilScaling_Routine = IDCT_BilinearScaling_90A;	
				IDCT_Scaling_Routine = IDCT_Scaling_90A;
				break;
			case ANG_180:
				hor_start_line += dst_w - 1;
				ver_start_line += dst_h - 1;
				IDCT_BilScaling_Routine = IDCT_BilinearScaling_180;	
				IDCT_Scaling_Routine = IDCT_Scaling_180;			
				break;
			case ANG_90_C:
				ver_start_line += dst_h - 1;
				IDCT_BilScaling_Routine = IDCT_BilinearScaling_90C;			
				IDCT_Scaling_Routine = IDCT_Scaling_90C;			
				break;
			default:				
				break;
		}
	}
	else
	{
		switch(g_imagedec_angle[phdl->id])
		{
			case ANG_90_A:
				hor_start_line += dst_w - 1 - blank_x;
				IDCT_BilScaling_Routine = IDCT_BilinearScaling_90A;	
				IDCT_Scaling_Routine = IDCT_Scaling_90A;
				break;
			case ANG_180:
				hor_start_line += dst_w - 1 - blank_x;
				ver_start_line += dst_h - 1 - blank_y;
				IDCT_BilScaling_Routine = IDCT_BilinearScaling_180;	
				IDCT_Scaling_Routine = IDCT_Scaling_180;			
				break;
			case ANG_90_C:
				ver_start_line += dst_h - 1 - blank_y;
				IDCT_BilScaling_Routine = IDCT_BilinearScaling_90C;			
				IDCT_Scaling_Routine = IDCT_Scaling_90C;			
				break;
			default:				
				break;
		}
	}
	//}
	if(hor_start_line&0x01)
		hor_start_line--;
	if(ver_start_line&0x01)
		ver_start_line--;
	//compute scale factors
	//integer scale factor is for scaling the picture to 736x540<=picture size<=1440x1080
	IDCT_mode_Y = 0;
	idct_row_num_Y = 8;
	idct_col_num_Y = 8;
	hor_scale_ratio_Y_int = 1;
	hor_scale_ratio_Y_fraction = 256;		
	ver_scale_ratio_Y_int = 1;
	ver_scale_ratio_Y_fraction = 256;		
	if(((decoded_picture_height < dst_h) || (decoded_picture_width < dst_w)) && (IMAGEDEC_REAL_SIZE != phdl->main.setting.mode))
	{
		jpeg_printf("not support scale up <%s>\n",__FUNCTION__);
		terminate(JPGD_NOT_SUPPORT_SCALE_UP);
	}
	if(decoded_picture_width != dst_w)
	{
		hor_scale_ratio_Y_int = 1;
		do
		{
			hor_scale_ratio_Y_int++;
			integer_scaled_width = decoded_picture_width/hor_scale_ratio_Y_int;
		}while(integer_scaled_width > dst_w);
		hor_scale_ratio_Y_int--;
		integer_scaled_width = decoded_picture_width/hor_scale_ratio_Y_int;
		hor_scale_ratio_Y_fraction = (integer_scaled_width<<8)/dst_w;
		if(hor_scale_ratio_Y_fraction * dst_w != (integer_scaled_width<<8))
			hor_scale_ratio_Y_fraction++;
		
		m_trans_hor_no_scale = 0;	
	}
	else
		m_trans_hor_no_scale = 1;

	if(decoded_picture_height != dst_h)
	{
#if 1
		ver_scale_ratio_Y_int = 1;
		do
		{
			ver_scale_ratio_Y_int++;
			integer_scaled_height = decoded_picture_height/ver_scale_ratio_Y_int;
		}while(integer_scaled_height > dst_h);
		ver_scale_ratio_Y_int--;
#else
		ver_scale_ratio_Y_int = hor_scale_ratio_Y_int;
#endif
		integer_scaled_height = decoded_picture_height/ver_scale_ratio_Y_int;
		ver_scale_ratio_Y_fraction = (integer_scaled_height<<8)/dst_h;
		if(ver_scale_ratio_Y_fraction * dst_h != (integer_scaled_height<<8))
			ver_scale_ratio_Y_fraction++;
		
		m_trans_ver_no_scale = 0;	
	}
	else
		m_trans_ver_no_scale = 1;
	scale_precision = 8;scale_precisionX2 = 16;
	scale_bits_field = 0xFF;
	scale_fra_total_value = 0x100;
	flag_very_large_picture = 0;
	if(ver_scale_ratio_Y_int > 1 || hor_scale_ratio_Y_int > 1)
	{
		if((ver_scale_ratio_Y_int > 10) || (hor_scale_ratio_Y_int > 10))
		{
			flag_very_large_picture = 1;
			hor_scale_ratio_Y_fraction = (integer_scaled_width<<4)/dst_w;
			ver_scale_ratio_Y_fraction = (integer_scaled_height<<4)/dst_h;
			scale_precision = 4;scale_precisionX2 = 8;
			scale_bits_field = 0xF;
			scale_fra_total_value = 0x10;			
			if(hor_scale_ratio_Y_fraction * dst_w != (integer_scaled_width<<4))
				hor_scale_ratio_Y_fraction++;
			if(ver_scale_ratio_Y_fraction * dst_h != (integer_scaled_height<<4))
				ver_scale_ratio_Y_fraction++;				
		}
	}
	//if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	//{
		if((ANG_90_A == g_imagedec_angle[phdl->id]) || (ANG_90_C == g_imagedec_angle[phdl->id]))
		{
			// switch the Ver and Hor Scale parameters
			int tmp = 0;

			tmp = hor_scale_ratio_Y_int;
			hor_scale_ratio_Y_int = ver_scale_ratio_Y_int;
			ver_scale_ratio_Y_int = tmp;

			tmp = hor_scale_ratio_Y_fraction;
			hor_scale_ratio_Y_fraction = ver_scale_ratio_Y_fraction;
			ver_scale_ratio_Y_fraction = tmp;
		}
	//}	
	jpeg_printf("scale para ver_int <%d>hor_int<%d>ver_f <%d>hor_f<%d>pos_x<%d>pos_y<%d>pre<%d>\n",ver_scale_ratio_Y_int
		,hor_scale_ratio_Y_int,ver_scale_ratio_Y_fraction,hor_scale_ratio_Y_fraction
		,hor_start_line,ver_start_line,scale_precision);
#if 0
	while(decoded_picture_height>=1080){ //change016
		IDCT_mode_Y++;

	#if 0 //frank enable this, the speed will be fast, but the image quality is not so good
		if((IDCT_mode_Y > 2)&&(scan_type!=JPGD_YH2V2)){ //change016
			//we can not do 1x1 IDCT if we have to do 2x1 IDCT for Cb Cr
			flag_very_large_picture = 1;
			IDCT_mode_Y--;
			break;
		}
		else if(IDCT_mode_Y > 3){ //this is only possbile for scan_type==JPGD_YH2V2
			flag_very_large_picture = 1;
			IDCT_mode_Y--;
			break;
		}
	#else // enable this, the speed will slower a little, but the image quality is better.
		if(IDCT_mode_Y > 1){
			flag_very_large_picture = 1;
			IDCT_mode_Y--;
			break;
		}
	#endif
//change017 
//manson: using IDCT size < 4x4 will result in some ringing artifacts. Although for most
//case such artifacts are not visible, however if you test with very close shot picture
//of hair or other sharp details you can see the artifacts. So here I limit the IDCT
//to 4x4. You can enable other small IDCT modes if the customer want to decode very 
//large pictures at very high speed and if they don't care about these artifacts. 
//I think we can reduce the artifacts by shifting the DCT domain decimation phase (right
//now it start from phase 0). But I don't have time to do it now. 
		else{
			flag_very_large_picture = 0;
			idct_row_num_Y = idct_row_num_Y >> 1;
			idct_col_num_Y = idct_col_num_Y >> 1;
			decoded_picture_height = decoded_picture_height >> 1;
			decoded_picture_width = decoded_picture_width >> 1;
		}
	}

	integer_scaled_height = decoded_picture_height;
	ver_scale_ratio_Y_int = 1;
	do{ //while(){..} will end up finding the first scale factor that result < 1080
		//do{...}while() end up finding the last one which is better
		ver_scale_ratio_Y_int++;
		integer_scaled_height = decoded_picture_height/ver_scale_ratio_Y_int;
	}while(integer_scaled_height>=540);
	ver_scale_ratio_Y_int--;
	integer_scaled_height = decoded_picture_height/ver_scale_ratio_Y_int;
//	libc_printf("integer_scaled_height:%d ver_scale_ratio_Y_int:%d\n",integer_scaled_height,ver_scale_ratio_Y_int);
	//change017
	//manson: if 540<=integer_scaled_height<576 in PAL output mode,
	//use the formular above will result in vertical scaling up which can not be done in the filter engine
	//so I treat it as a special case below
	if((integer_scaled_height>=540)&&(integer_scaled_height<TV_ver_lines)&&(ver_scale_ratio_Y_int>1)){
		flag_very_large_picture = 1; //this will force using the filter engine that can handle scale factor > 2.0
		ver_scale_ratio_Y_int--;
		integer_scaled_height = decoded_picture_height/ver_scale_ratio_Y_int;
	}

	hor_scale_ratio_Y_int = ver_scale_ratio_Y_int;//horizontal integer scale factor is the same as vertical
	integer_scaled_width = decoded_picture_width/ver_scale_ratio_Y_int; 
	//fractional scale factor is for scaling and pixel aspect ratio convertion to 736x480 or 736x576
	//use 8 bits to represent the fraction, combine scaling with pixel aspect ratio convertion
	hor_scale_ratio_Y_fraction = (integer_scaled_width<<8)/720;  //TV_ver_lines = 576 or 480
	ver_scale_ratio_Y_fraction = (integer_scaled_height<<8)/TV_ver_lines;  

	if((hor_scale_ratio_Y_fraction*720) != (integer_scaled_width<<8))
		hor_scale_ratio_Y_fraction = hor_scale_ratio_Y_fraction + 1;
	if((ver_scale_ratio_Y_fraction*TV_ver_lines) != (integer_scaled_height<<8))
		ver_scale_ratio_Y_fraction = ver_scale_ratio_Y_fraction + 1; //+1 to cover round off case
	flag_need_scaling_up = 0;
	
	if(org_height < TV_ver_lines){
		//for scaling up, force the scaling factor to 1.0 because scaling up is done seperately
		ver_scale_ratio_Y_int = 1;
		ver_scale_ratio_Y_fraction = 256;
		flag_need_scaling_up = 1;
		if(IMAGEDEC_THUMBNAIL != phdl->main.setting.mode)
		{
			ver_start_line = (TV_ver_lines - org_height)>>1;
			if(ver_start_line&0x01)
				ver_start_line--;
		}
	}
	if(org_width < 720){
		hor_scale_ratio_Y_int = 1;
		hor_scale_ratio_Y_fraction = 256;
		flag_need_scaling_up = 1;
		if(IMAGEDEC_THUMBNAIL != phdl->main.setting.mode)
		{
			hor_start_line = (TV_hor_columns - org_width)>>1;
			if(hor_start_line&0x01)
				hor_start_line--;
		}
	}
#endif
	//because linear interpolation (fraction scaling) doesn't accumulate signal power
	//so we calculate inverse scale factor only with the integer part

	scale_factor_inverse_2D_Y = 32768/(hor_scale_ratio_Y_int*ver_scale_ratio_Y_int);
	scale_factor_inverse_2D_Bits = 15;
	if(IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)
	{
		if(1 == flag_very_large_picture)
		{
			scale_factor_inverse_2D_Y = hor_scale_ratio_Y_int*ver_scale_ratio_Y_int;
			scale_factor_inverse_2D_Bits = 0;
		}
	}
	ver_final_Y_index_MCU = ver_start_line;
	ver_final_Y_index_block = ver_start_line;
	hor_final_Y_index_MCU = hor_start_line; 
	hor_final_Y_index_block = hor_start_line; 
	hor_final_Cb_index_block = hor_start_line>>1; 
	ver_final_Cb_index_block = ver_start_line>>1;
	hor_final_Cr_index_block = hor_final_Cb_index_block; 
	ver_final_Cr_index_block = ver_final_Cb_index_block;
#if 0	
	decoded_picture_real_height_Y = org_height >> IDCT_mode_Y;
	decoded_picture_real_width_Y = org_width >> IDCT_mode_Y;
#else
	//if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	//{
		if(g_imagedec_m33_combine_hw_sw_flag)
		{
			if(0)//(ANG_90_A == g_imagedec_angle[phdl->id]) || (ANG_90_C == g_imagedec_angle[phdl->id]))
			{
				decoded_picture_real_height_Y = phdl->imageout.hw_acce_rect.uWidth >> IDCT_mode_Y;
				decoded_picture_real_width_Y = phdl->imageout.hw_acce_rect.uHeight >> IDCT_mode_Y;
			}
			else
			{
				decoded_picture_real_height_Y = phdl->imageout.hw_acce_rect.uHeight >> IDCT_mode_Y;
				decoded_picture_real_width_Y = phdl->imageout.hw_acce_rect.uWidth >> IDCT_mode_Y;
			}			
		}
		else
		{
			if((ANG_90_A == g_imagedec_angle[phdl->id]) || (ANG_90_C == g_imagedec_angle[phdl->id]))
			{
				decoded_picture_real_height_Y = org_width >> IDCT_mode_Y;
				decoded_picture_real_width_Y = org_height >> IDCT_mode_Y;
			}
			else
			{
				decoded_picture_real_height_Y = org_height >> IDCT_mode_Y;
				decoded_picture_real_width_Y = org_width >> IDCT_mode_Y;
			}
		}
		
	//}
#endif	

	//set some parameters' value, they may change in some case below
	flag_CbCr_Hor_copy = 0; 
	hor_scale_ratio_CbCr_int = hor_scale_ratio_Y_int; 
	ver_scale_ratio_CbCr_int = ver_scale_ratio_Y_int; 
	hor_scale_ratio_CbCr_fraction = hor_scale_ratio_Y_fraction;  
	ver_scale_ratio_CbCr_fraction = ver_scale_ratio_Y_fraction; 
	scale_factor_inverse_2D_CbCr = scale_factor_inverse_2D_Y;
	switch (phdl->decoder.out_type){
	case JPGD_YH2V2: //4:2:0 
		IDCT_mode_CbCr = IDCT_mode_Y;
		idct_row_num_CbCr = idct_row_num_Y; 
		idct_col_num_CbCr = idct_col_num_Y; 
		MCU_ver_Y_block_num = 2;
		MCU_hor_Y_block_num = 2; //change014
		decoded_picture_real_height_CbCr = decoded_picture_real_height_Y >> 1; //change014
		decoded_picture_real_width_CbCr = decoded_picture_real_width_Y >> 1; //change014
		break;
	case JPGD_YH2V1: //4:2:2
		IDCT_mode_CbCr = IDCT_mode_Y + 8; 
		if(IDCT_mode_CbCr > 10)
			return; //picture too large
		idct_row_num_CbCr = idct_row_num_Y >> 1; 
		idct_col_num_CbCr = idct_col_num_Y; 
		MCU_ver_Y_block_num = 1;
		MCU_hor_Y_block_num = 2; //change014
		decoded_picture_real_height_CbCr = decoded_picture_real_height_Y>>1;//change014
		decoded_picture_real_width_CbCr = decoded_picture_real_width_Y>>1;//change014
		break;
	case JPGD_YH1V2: //vertically decimated by 2, horizontal the same as Y
		//I don't want to spend my time to write a 4x8 IDCT
		//but it is not necessary to optimize since it is not popular anyway
		IDCT_mode_CbCr = IDCT_mode_Y;
		idct_row_num_CbCr = idct_row_num_Y;
		idct_col_num_CbCr = idct_col_num_Y;
		hor_scale_ratio_CbCr_int = hor_scale_ratio_Y_int << 1; 
		if(0 == scale_factor_inverse_2D_Bits)
			scale_factor_inverse_2D_CbCr = scale_factor_inverse_2D_Y << 1;
		else
			scale_factor_inverse_2D_CbCr = scale_factor_inverse_2D_Y >> 1;
		MCU_ver_Y_block_num = 2;
		MCU_hor_Y_block_num = 1; //change014
		decoded_picture_real_height_CbCr = decoded_picture_real_height_Y >> 1;//change014
		decoded_picture_real_width_CbCr = decoded_picture_real_width_Y;//change014
		break;
	case JPGD_YH4V1: //4:1:1
		MCU_ver_Y_block_num = 1; //change014
		MCU_hor_Y_block_num = 4; //change014
		if((IDCT_mode_Y==1)||(IDCT_mode_Y==2)){
			IDCT_mode_CbCr = IDCT_mode_Y + 16;
			idct_row_num_CbCr = idct_row_num_Y >> 1; 
			idct_col_num_CbCr = idct_col_num_Y << 1; 
			decoded_picture_real_height_CbCr = decoded_picture_real_height_Y >> 1;//change014
			decoded_picture_real_width_CbCr = decoded_picture_real_width_Y >> 1;
		}
		else if(IDCT_mode_Y==0){
			IDCT_mode_CbCr = 8; //8x4
			flag_CbCr_Hor_copy = 1;
			idct_row_num_CbCr = idct_row_num_Y >> 1; 
			idct_col_num_CbCr = idct_col_num_Y << 1;  //after copying
			decoded_picture_real_height_CbCr = decoded_picture_real_height_Y >> 1;//change014
			decoded_picture_real_width_CbCr = decoded_picture_real_width_Y >> 1;
		}
		else{ //(IDCT_mode_Y==3)
			IDCT_mode_CbCr = 10; //2x1 
			ver_scale_ratio_CbCr_int = ver_scale_ratio_Y_int * 2; //vertical decimate by 2
			idct_row_num_CbCr = idct_row_num_Y; 
			idct_col_num_CbCr = idct_col_num_Y << 1;  //after copying
			decoded_picture_real_height_CbCr = decoded_picture_real_height_Y;
			decoded_picture_real_width_CbCr = decoded_picture_real_width_Y>>1;
		}
		MCU_ver_Y_block_num = 1;
		break;
	case JPGD_YH1V1: //4:4:4
		MCU_ver_Y_block_num = 1; //change014
		MCU_hor_Y_block_num = 1; //change014
		IDCT_mode_CbCr = IDCT_mode_Y+1;
		if(IDCT_mode_CbCr==4)
			return; //picture too large
		idct_row_num_CbCr = idct_row_num_Y >> 1; 
		idct_col_num_CbCr = idct_col_num_Y >> 1; 
		decoded_picture_real_height_CbCr = decoded_picture_real_height_Y >> 1;
		decoded_picture_real_width_CbCr = decoded_picture_real_width_Y >> 1;
		MCU_ver_Y_block_num = 1;
		break;
	case JPGD_GRAYSCALE:
		//fill in CbCr with black
		MCU_ver_Y_block_num = 1; //change014
		MCU_hor_Y_block_num = 1; //change014
/* //change014
		for(ver_counter=0;ver_counter<288;ver_counter++){
			for(hor_counter=0;hor_counter<360;hor_counter++){
				final_output_buf_Cb[ver_counter*Final_Output_Buf_Stride_CbCr+hor_counter] = 128;
				final_output_buf_Cr[ver_counter*Final_Output_Buf_Stride_CbCr+hor_counter] = 128;
			}
		}
*/
		MCU_ver_Y_block_num = 1;
		break;
	}

	if(1 == m_trans_hor_no_scale)
	{
		hor_integer_counter_Y1 = 1; //bugfix002//counter for horizontal integer scaling output 
		hor_integer_counter_Y2 = 1; //bugfix002//counter for horizontal integer scaling output 
		hor_fraction_counter_Y1 = 0;//hor_scale_ratio_Y_fraction;//0; //change015
		hor_fraction_counter_Y2 = 0;//hor_scale_ratio_Y_fraction;//0; //change015
		hor_integer_counter_Cb = 1; //counter for horizontal integer scaling output 
		hor_fraction_counter_Cb = 0;//hor_scale_ratio_CbCr_fraction;//0; //change015
		hor_integer_counter_Cr = 1; //counter for horizontal integer scaling output 
		hor_fraction_counter_Cr = 0;//hor_scale_ratio_CbCr_fraction;//0; //change015
	}
	else
	{
		hor_integer_counter_Y1 = 0; //bugfix002//counter for horizontal integer scaling output 
		hor_integer_counter_Y2 = 0; //bugfix002//counter for horizontal integer scaling output 
		hor_fraction_counter_Y1 = hor_scale_ratio_Y_fraction;//0; //change015
		hor_fraction_counter_Y2 = hor_scale_ratio_Y_fraction;//0; //change015
		hor_integer_counter_Cb = 0; //counter for horizontal integer scaling output 
		hor_fraction_counter_Cb = hor_scale_ratio_CbCr_fraction;//0; //change015
		hor_integer_counter_Cr = 0; //counter for horizontal integer scaling output 
		hor_fraction_counter_Cr = hor_scale_ratio_CbCr_fraction;//0; //change015
		
	}

	if(1 == m_trans_ver_no_scale)
	{
		row_ver_integer_counter_Y = 1; //counter for vertical integer scaling output 
		row_ver_fraction_counter_Y = 0;//ver_scale_ratio_Y_fraction; //counter for vertical fractional scaling
		row_ver_integer_counter_Cb = 1; //counter for vertical integer scaling output 
		row_ver_fraction_counter_Cb = 0;//ver_scale_ratio_CbCr_fraction; //counter for vertical fractional scaling
		row_ver_integer_counter_Cr = 1; //counter for vertical integer scaling output 
		row_ver_fraction_counter_Cr = 0;//ver_scale_ratio_CbCr_fraction; //counter for vertical fractional scaling
	}
	else
	{
		row_ver_integer_counter_Y = 0; //counter for vertical integer scaling output 
		row_ver_fraction_counter_Y = ver_scale_ratio_Y_fraction; //counter for vertical fractional scaling
		row_ver_integer_counter_Cb = 0; //counter for vertical integer scaling output 
		row_ver_fraction_counter_Cb = ver_scale_ratio_CbCr_fraction; //counter for vertical fractional scaling
		row_ver_integer_counter_Cr = 0; //counter for vertical integer scaling output 
		row_ver_fraction_counter_Cr = ver_scale_ratio_CbCr_fraction; //counter for vertical fractional scaling
	}
	
	flag_hor_hiding_line_added = 0;
	flag_hor_hiding_line_added_Y = 0;
	flag_hor_hiding_line_added_Cb = 0;
	flag_hor_hiding_line_added_Cr = 0;
	decoded_picture_height_counter_Y = idct_row_num_Y * MCU_ver_Y_block_num; //change014
	decoded_picture_height_counter_Cb = idct_row_num_CbCr;
	decoded_picture_height_counter_Cr = idct_row_num_CbCr;
	//decoded_picture_height_counter_Y = 0;
	//decoded_picture_height_counter_Cb = 0;
	//decoded_picture_height_counter_Cr = 0;
	decoded_picture_width_counter_Y = 0;  //change014
	decoded_picture_width_counter_Cb = 0;
	decoded_picture_width_counter_Cr = 0;
	idct_row_num_Y1 = idct_row_num_Y;
	idct_row_num_Y2 = idct_row_num_Y;

	idct_col_num_Y_ForMost = idct_col_num_Y; //change014, column number for most MCU blocks
	idct_col_num_CbCr_ForMost = idct_col_num_CbCr; //change014, column number for most MCU blocks
	flag_is_last_hor_MCU = 0;
	MCU_output_ver_count = 0; //change017
	max_MCU_output_ver_count_Y = 0; //change018, if the picture size is very large, we don't know which MCU has vertical output, so 

	MCU_output_hor_count = 0; 
	max_MCU_output_hor_count_Y = 0; 
	
	//change014
	tmp = hor_scale_ratio_CbCr_int<<7;
	for(i=0;i<16;i++){ 
		hor_partsum_Y[i] = 0;
		hor_partsum_Cb[i] = 0;
		hor_partsum_Cr[i] = 0;
		hor_bilinear_Y1[i] = 0;
		hor_bilinear_Cb1[i] = tmp;//128 * hor_scale_ratio_CbCr_int;
		hor_bilinear_Cr1[i] = tmp;//128 * hor_scale_ratio_CbCr_int;
	}

	if(g_imagedec_m33_combine_hw_sw_flag)
	{
		for(i=0;i<JPEG_HW_MAX_SCALED_WIDTH_EXT;i++){
			ver_partsum_Y[i] = 0; 
			ver_bilinear_Y1[i] = 0;
		}
		
		tmp = (hor_scale_ratio_CbCr_int * ver_scale_ratio_CbCr_int)<<15;
		for(i=0;i<JPEG_HW_MAX_SCALED_WIDTH_EXT/2;i++){
			ver_partsum_Cb[i] = 0; 
			ver_bilinear_Cb1[i] = tmp;//(128 * hor_scale_ratio_CbCr_int * ver_scale_ratio_CbCr_int)<<8; 
		}
		
		for(i=0;i<JPEG_HW_MAX_SCALED_WIDTH_EXT/2;i++){
			ver_partsum_Cr[i] = 0; 
			ver_bilinear_Cr1[i] = tmp;//(128 * hor_scale_ratio_CbCr_int * ver_scale_ratio_CbCr_int)<<8;  
		}
	}
	else
	{
		for(i=0;i<720;i++){
			ver_partsum_Y[i] = 0; 
			ver_bilinear_Y1[i] = 0;
		}
		
		tmp = (hor_scale_ratio_CbCr_int * ver_scale_ratio_CbCr_int)<<15;
		for(i=0;i<360;i++){
			ver_partsum_Cb[i] = 0; 
			ver_bilinear_Cb1[i] = tmp;//(128 * hor_scale_ratio_CbCr_int * ver_scale_ratio_CbCr_int)<<8; 
		}
		
		for(i=0;i<360;i++){
			ver_partsum_Cr[i] = 0; 
			ver_bilinear_Cr1[i] = tmp;//(128 * hor_scale_ratio_CbCr_int * ver_scale_ratio_CbCr_int)<<8;  
		}
	}
}

static int com_row_num;
static void combine_scale_mcu(pImagedec_hdl phdl)
{
	int i = 0, j = 0, m = 0;
	int stride = phdl->imageout.hw_acc_stride;
	UINT32 y_addr = (UINT32)phdl->imageout.hw_acc_frm_y - 0x20000000;
	UINT32 c_addr = (UINT32)phdl->imageout.hw_acc_frm_c - 0x20000000;
	UINT8 *src_addr = NULL;
	int *dst_addr = temp_IDCT_buf2;
	int y_data = 0;
	
	for(i = 0;i < phdl->decoder.out_blocks_per_mcu;i++)	
	{
		// prepare the raw video block data for the temp_IDCT_buf2
		dst_addr = temp_IDCT_buf2;
		switch (phdl->decoder.out_type)
		{
			case JPGD_YH2V1:
				if(2 == i || 3 == i)
				{
					y_data = 0;
					if(2 == i)
					{
						c_addr += ((com_row_num & 0xFE)<<3) * stride + ((com_row_num & 1)<<7);
						c_addr += phdl->imageout.combine_hw_mcu_row_cnt<<8;
					}

					src_addr = (UINT8 *)((2 == i) ? c_addr : c_addr + 1);				
				}
				else
				{
					y_data = 1;
					if(0 == i)
					{
						y_addr += ((com_row_num & 0xFE)<<3) * stride + ((com_row_num & 1)<<7);	
						y_addr += phdl->imageout.combine_hw_mcu_row_cnt<<8;

						src_addr = (UINT8 *)(y_addr);
					}
					else
						src_addr = (UINT8 *)(y_addr + 8);				
				}								
				break;
			case JPGD_YH2V2:
				if(4 == i || 5 == i)
				{
					y_data = 0;
					if(4 == i)
					{
						c_addr += ((com_row_num & 0xFE)<<3) * stride + ((com_row_num & 1)<<7);
						c_addr += phdl->imageout.combine_hw_mcu_row_cnt<<8;
					}

					src_addr = (UINT8 *)((4 == i) ? c_addr : c_addr + 1);
				}
				else
				{
					y_data = 1;
					if(0 == i)
					{
						y_addr += (com_row_num<<4) * stride;
						y_addr += phdl->imageout.combine_hw_mcu_row_cnt<<8;
					}
					
					src_addr = (UINT8 *)(y_addr + ((i&2)<<6) + ((i&1)<<3));
				}				
				break;
			case JPGD_YH4V1:
				if(4 == i || 5 == i)
				{
					y_data = 0;
					if(4 == i)
					{
						c_addr += ((com_row_num & 0xFE)<<3) * (stride>>1) + ((com_row_num & 1)<<7);
						c_addr += phdl->imageout.combine_hw_mcu_row_cnt<<8;
					}						
					
					src_addr = (UINT8 *)((4 == i) ? c_addr : c_addr + 1);
				}
				else
				{
					y_data = 1;
					if(0 == i)
					{
						y_addr += ((com_row_num & 0xFE)<<3) * stride + ((com_row_num & 1)<<7);	
						y_addr += phdl->imageout.combine_hw_mcu_row_cnt<<9;
					}	
					
					src_addr = (UINT8 *)(y_addr + ((i&2)<<7) + ((i&1)<<3));
				}				
				break;
			default:
				return;
		}

		osal_cache_invalidate(src_addr, 256);	
		if(y_data)
		{		
			for(j = 0;j < 8;j++)
			{
				for(m = 0;m < 8;m++)
					*dst_addr++ = (int)(*src_addr++);
				
				src_addr += 8;
			}
		}
		else
		{		
			if(8 == idct_row_num_CbCr)
			{
				for(j = 0;j < 8;j++)
				{			
					for(m = 0;m < 8;m++)
					{
						*dst_addr++ = (int)*src_addr;
						src_addr += 2;
					}
				}
			}
			else if(4 == idct_row_num_CbCr)
			{
				for(j = 0;j < 4;j++)
				{			
					for(m = 0;m < 8;m++)
					{
						*dst_addr++ = (int)*src_addr;
						src_addr += 2;
					}
					src_addr += 16;
				}

			}
		}
		
		IDCT_Scaling_Routine(phdl,0);
	}
}

void combine_sw_scale_row(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];	
	int col_num = 0;
	int i = 0;

	switch (phdl->decoder.out_type)
	{
		case JPGD_YH2V1:
			col_num = JPEG_HW_MacroBlock_NUM * 2;
			break;
		case JPGD_YH2V2:
			col_num = JPEG_HW_MacroBlock_NUM * 1;
			break;
		case JPGD_YH4V1:
			col_num = JPEG_HW_MacroBlock_NUM * 2;
			break;
		default:
			return;
	}

	for(com_row_num = 0;com_row_num < col_num;com_row_num++)
	{
		phdl->imageout.combine_hw_mcu_row_cnt = 0;
		for(i = 0;i < phdl->imageout.combine_hw_mcu_row_max;i++)
		{
			combine_scale_mcu(phdl);
			phdl->imageout.combine_hw_mcu_row_cnt++;			
		}

		PrepareForNextRow(0);
	}
}

//////////////////////////////////////////////////////////
//manson: rotate the picture store in one frame buffer by 90 degree, clockwise
//note that pixel aspect ratio must be converted
void Rotate270()
{
//algorithm explanation:
//since we want to see the whole picture after rotation, so the original width 720
//will always be converted to 480 (or 576). If we ignore the pixel aspect ratio issue
//the original height 480 will be converted to 480 * 480/720.
//correction for the statement above: but some picture are vertically encoded (height 
//longer than width, we shall only convert the real width to 480/576. But the width
//may be too short that after rotation we still need adding horizontal lines. 
//now we consider the pixel aspect ration, take NTSC as example, orginal picture 
//is 736x480, and this is displayed as 4x3, so pixel aspect ratio is 
//720/480 * asp_r = 4/3 => asp_r = 4/3 * 480/720
//after rotation, asp_r = 1/asp_r, so the new width should be interpolated by
//(3/4 * 720/480)^2
//so the total scaling factor for height to width convertion is 
//480/720 * (3/4 * 720/480)^2 = 9/16 * 720/480 = 0.84375

//The interpolation is a input driven process. It is done in the steps below.
//1) Two column buffers are used in a ping-pong way, one stores the current
//   row to column convertion, the others stores the previous row to column
//   convertion. Swap the buffer points when a new column is generated
//2) Process the original picture row by row (row to column convertion). 
//   Each row generate one column output, store it in the current column buffer. 
//3) After one new column is generated, check is it ready to do column to row 
//   convertion (interpolation between the new column and the previous column. 
//   If it is ready, do linear interpolation and write output to rotated picture 
//   frame buffer
//4) Swap column buffer pointer. So the current column become previous column 
//   and the previous column is discarded. 
//   Row to Column means Row length to new column length, Column to Row means column length to new row length

//Note that rotation takes the whole input frame buffer (including added black
//lines for 4:3 aspect ratio) as valid picture. So the added black lines are 
//also rotated
#if 0 //john
	int i;
	int width_to_heigh_ratio;
	int height_to_width_ratio; 
	int *pOutputcolumnNew;
	int *pOutputcolumnOld;
	int *pSwap;
	int *pTempNew;
	int *pTempOld;
	int *pTempNew2;
	int *pTempOld2;

	uchar *pCurnInputRow;
	uchar *pCurnInput;
	uchar *pRotated_output_col; //pointer to the current rotated output column
	uchar *pCurn_rotate_output; //pointer to the current rotated output sample
	int hor_counter;
	int ver_counter;
	int first_column;
	int last_column;
	int temp_result;
	int fraction_counter_r2c; 
	int fraction_counter_c2r; 
	int fraction;
	int fraction2;
	int r2c_counter;
	int c2r_counter;
	int stride_index;
	int output_columns;
	int add_black_lines;
	int old_width;
	int old_height;
	int output_rows;
	int first_row;
	int last_row;
	int flag_add_black_row;
	int flag_add_black_column;
	int temp;
	int row_index1;
	int row_index2;
	char rotate_temp;
	int read_checksumY1;
	int read_checksumY2;
	int read_checksumCb1;
	int read_checksumCb2;
	int read_checksumCr1;
	int read_checksumCr2;
	int write_checksumY1;
	int write_checksumY2;
	int write_checksumCb1;
	int write_checksumCb2;
	int write_checksumCr1;
	int write_checksumCr2;
	int try_times;
	int max_try_time;
	uchar curn_input; 
	int output_column_counter;
	uchar *pFirst_Rotated_output;
	int row_counter;
	int col_counter;
	uchar *pRotated_output_row;

	if(current_hor_start_line==0){ //the picutre takes full 720 columns)
		old_width = 720;
		flag_add_black_row = 0;
		flag_add_black_column = 1;
		width_to_heigh_ratio = 32768 * 720 / TV_ver_lines; 
		height_to_width_ratio = (32768 * 16 * TV_ver_lines)/(9 * 720);
		output_columns = TV_ver_lines  * 32768 / height_to_width_ratio;
		output_rows = TV_ver_lines;
		add_black_lines = 720 - output_columns; 
		first_column = add_black_lines >> 1;
		first_column = first_column & 0xfffffffe;
		last_column = 720-first_column-4; //only used in adding black lines
	}
	else{ //check how the picture can fit in the rotated window
//check if we convert the old width to TV_ver_lines, to maintain the correct
//aspect ratio, does the old height (TV_ver_lines) fit into 720 lines?
		old_width = current_hor_stop_line - current_hor_start_line;
		old_height = current_ver_stop_line  - current_ver_start_line;
		width_to_heigh_ratio = 32768 * old_width / TV_ver_lines; 
		//height_to_width_ratio = (width_to_heigh_ratio * 16 * TV_ver_lines * TV_ver_lines) / (720*720*9)
		temp = (16 * TV_ver_lines * TV_ver_lines) / 900;
		height_to_width_ratio = (temp * width_to_heigh_ratio) / (72 * 72);
		//output_columns = TV_ver_lines  * 32768 / height_to_width_ratio;
		output_columns = old_height  * 32768 / height_to_width_ratio;
		if(output_columns > 720){ 
			//we have to fit old height in new 720 width and add horizontal black lines
			flag_add_black_row = 1;
			flag_add_black_column = 0;
			height_to_width_ratio = 32768 * TV_ver_lines / 720;
			//converting from height to width is the reverse of the converting 
			//from width to height formular above
			//width_to_heigh_ratio = (9 *  720 * 720 * height_to_width_ratio)/(16*TV_ver_lines * TV_ver_lines
			//split into two steps to avoid overflow
			temp = 9*720*height_to_width_ratio/(16*TV_ver_lines); //change 013
			width_to_heigh_ratio = temp*720/TV_ver_lines;
			//width_to_heigh_ratio = 32768 * 720 * 9 / (16 * TV_ver_lines);
			//output_rows = 720 * 32768 / width_to_heigh_ratio;
			output_rows = old_width * 32768 / width_to_heigh_ratio;
			output_columns = 720;
			add_black_lines = TV_ver_lines - output_rows;
			first_row = add_black_lines >> 1;
			first_row = (first_row+1) & 0xfffffffe;   //change013
			last_row = TV_ver_lines - first_row;
		}
		else{
			output_rows = TV_ver_lines;
			flag_add_black_row = 0;
			flag_add_black_column = 1;
			add_black_lines = 720 - output_columns; 
			first_column = add_black_lines >> 1;
			first_column = (first_column+1) & 0xfffffffe;
			last_column = 720-first_column; //only used in adding black lines
		}
	}
	//add black lines
/*
	if(flag_add_black_column!=0){
		stride_index = 0;
		for(ver_counter=0;ver_counter<TV_ver_lines;ver_counter++){
			for(hor_counter=0;hor_counter<first_column;hor_counter++){
				rotated_output_buf_Y[stride_index + hor_counter] = 0;
			}
			for(hor_counter=last_column;hor_counter<720;hor_counter++){
				rotated_output_buf_Y[stride_index + hor_counter] = 0;
			}
			stride_index = stride_index+Final_Output_Buf_Stride_Y;
		}
		stride_index = 0;
		for(ver_counter=0;ver_counter<(TV_ver_lines>>1);ver_counter++){
			for(hor_counter=0;hor_counter<(first_column>>1);hor_counter++){
				rotated_output_buf_Cb[stride_index + hor_counter] = 128;
				rotated_output_buf_Cr[stride_index + hor_counter] = 128;
			}
			for(hor_counter=(last_column>>1);hor_counter<360;hor_counter++){
				rotated_output_buf_Cb[stride_index + hor_counter] = 128;
				rotated_output_buf_Cr[stride_index + hor_counter] = 128;
			}
			stride_index = stride_index+Final_Output_Buf_Stride_CbCr;
		}
	}
	if(flag_add_black_row!=0){
		stride_index = 0;
		for(ver_counter=0;ver_counter<=first_row;ver_counter++){ //= to cover round off case
			for(hor_counter=0;hor_counter<720;hor_counter++){
				rotated_output_buf_Y[stride_index + hor_counter] = 0;
			}
			stride_index = stride_index+Final_Output_Buf_Stride_Y;
		}
		stride_index = last_row * Final_Output_Buf_Stride_Y;
		for(ver_counter=last_row;ver_counter<TV_ver_lines;ver_counter++){
			for(hor_counter=0;hor_counter<720;hor_counter++){
				rotated_output_buf_Y[stride_index + hor_counter] = 0;
			}
			stride_index = stride_index+Final_Output_Buf_Stride_Y;
		}
		stride_index = 0;
		for(ver_counter=0;ver_counter<=(first_row>>1);ver_counter++){
			for(hor_counter=0;hor_counter<360;hor_counter++){
				rotated_output_buf_Cb[stride_index + hor_counter] = 128;
				rotated_output_buf_Cr[stride_index + hor_counter] = 128;
			}
			stride_index = stride_index+Final_Output_Buf_Stride_CbCr;
		}
		stride_index = (last_row >> 1) * Final_Output_Buf_Stride_CbCr;
		for(ver_counter=(last_row>>1);ver_counter<=(TV_ver_lines>>1);ver_counter++){
			for(hor_counter=0;hor_counter<360;hor_counter++){
				rotated_output_buf_Cb[stride_index + hor_counter] = 128;
				rotated_output_buf_Cr[stride_index + hor_counter] = 128;
			}
			stride_index = stride_index+Final_Output_Buf_Stride_CbCr;
		}
	}
*/
	width_to_heigh_ratio = width_to_heigh_ratio + 1; //+1 to cover round off case 
	height_to_width_ratio = height_to_width_ratio + 1; //change011


	//change019, for some unknown reason, reading or writing uncacheable memory may result in error
	//here I detect the error and re-do the rotation if error happens
	//now detect error in reading from uncacheable memory
	max_try_time = 5;
	read_checksumY1 = 0;
	read_checksumY2 = 0;
	for(c2r_counter=0;c2r_counter<TV_ver_lines;c2r_counter++){
		//pCurnInputRow = &final_output_buf_Y[c2r_counter*Final_Output_Buf_Stride_Y];
		pCurnInputRow = &pDecFBPic->pYAddr[c2r_counter*Final_Output_Buf_Stride_Y];
		pCurnInput = pCurnInputRow+current_hor_start_line;
		for(r2c_counter=0;r2c_counter<old_width;r2c_counter++){
			read_checksumY1 = read_checksumY1 + *pCurnInput;
			pCurnInput++;
		}
	}
	for(try_times=0;try_times<max_try_time;try_times++){
		for(c2r_counter=0;c2r_counter<TV_ver_lines;c2r_counter++){
			//pCurnInputRow = &final_output_buf_Y[c2r_counter*Final_Output_Buf_Stride_Y];
			pCurnInputRow = &pDecFBPic->pYAddr[c2r_counter*Final_Output_Buf_Stride_Y];
			pCurnInput = pCurnInputRow+current_hor_start_line;
			for(r2c_counter=0;r2c_counter<old_width;r2c_counter++){
				read_checksumY2 = read_checksumY1 + *pCurnInput;
				pCurnInput++;
			}
		}
		if(read_checksumY2==read_checksumY1)
			break;
		else{
			read_checksumY1 = read_checksumY2; //try until two continuous check sum is found
			read_checksumY2 = 0;
			data_error_counter++;
		}
	}
	read_checksumCb1 = 0;
	read_checksumCb2 = 0;
	for(c2r_counter=0;c2r_counter<(TV_ver_lines>>1);c2r_counter++){
		//pCurnInputRow = &final_output_buf_Cb[c2r_counter*Final_Output_Buf_Stride_CbCr];
		pCurnInputRow = &pDecFBPic->pCAddr[c2r_counter*Final_Output_Buf_Stride_CbCr];
		pCurnInput = pCurnInputRow+(current_hor_start_line>>1);
		for(r2c_counter=0;r2c_counter<(old_width>>1);r2c_counter++){
			read_checksumCb1 = read_checksumCb1 + *pCurnInput;
			pCurnInput++;
		}
	}
	for(try_times=0;try_times<max_try_time;try_times++){
		for(c2r_counter=0;c2r_counter<(TV_ver_lines>>1);c2r_counter++){
			//pCurnInputRow = &final_output_buf_Cb[c2r_counter*Final_Output_Buf_Stride_CbCr];
			pCurnInputRow = &pDecFBPic->pCAddr[c2r_counter*Final_Output_Buf_Stride_CbCr];
			pCurnInput = pCurnInputRow+(current_hor_start_line>>1);
			for(r2c_counter=0;r2c_counter<(old_width>>1);r2c_counter++){
				read_checksumCb2 = read_checksumCb2 + *pCurnInput;
				pCurnInput++;
			}
		}
		if(read_checksumCb2==read_checksumCb1)
			break;
		else{
			read_checksumCb1 = read_checksumCb2; //try until two continuous check sum is found
			read_checksumCb2 = 0;
			data_error_counter++;
		}
	}

	read_checksumCr1 = 0;
	read_checksumCr2 = 0;
	for(c2r_counter=0;c2r_counter<(TV_ver_lines>>1);c2r_counter++){
		//pCurnInputRow = &final_output_buf_Cr[c2r_counter*Final_Output_Buf_Stride_CbCr];
		pCurnInputRow = &pDecFBPic->pCAddr[c2r_counter*Final_Output_Buf_Stride_CbCr];
		pCurnInput = pCurnInputRow+(current_hor_start_line>>1);
		for(r2c_counter=0;r2c_counter<(old_width>>1);r2c_counter++){
			read_checksumCr1 = read_checksumCr1 + *pCurnInput;
			pCurnInput++;
		}
	}
	for(try_times=0;try_times<max_try_time;try_times++){
		for(c2r_counter=0;c2r_counter<(TV_ver_lines>>1);c2r_counter++){
			//pCurnInputRow = &final_output_buf_Cr[c2r_counter*Final_Output_Buf_Stride_CbCr];
			pCurnInputRow = &pDecFBPic->pCAddr[c2r_counter*Final_Output_Buf_Stride_CbCr];
			pCurnInput = pCurnInputRow+(current_hor_start_line>>1);
			for(r2c_counter=0;r2c_counter<(old_width>>1);r2c_counter++){
				read_checksumCr2 = read_checksumCr2 + *pCurnInput;
				pCurnInput++;
			}
		}
		if(read_checksumCr2==read_checksumCr1)
			break;
		else{
			read_checksumCr1 = read_checksumCr2; //try until two continuous check sum is found
			read_checksumCr2 = 0;
			data_error_counter++;
		}
	}
	//now read_checksumY1 read_checksumCb1 read_checksumCr1 are most likely correct
	//now start converting
	//rotate Y
	for(try_times=0;try_times<max_try_time;try_times++){
		read_checksumY2 = 0;
		write_checksumY1 = 0;
		output_column_counter = 0;
		pOutputcolumnNew = line_buffer_1;
		pOutputcolumnOld = line_buffer_2;
		for(i=0;i<576;i++){
			line_buffer_1[i] = 0;
			line_buffer_2[i] = 0;
		}
		fraction_counter_c2r = 0;
		if(flag_add_black_row!=0){
			//The original picture is vertically too long. 
			pRotated_output_col = &rotated_output_buf_Y[last_row*Final_Output_Buf_Stride_Y];
		}
		else{
			pRotated_output_col = &rotated_output_buf_Y[(TV_ver_lines-1)*Final_Output_Buf_Stride_Y+first_column];
		}
		pFirst_Rotated_output = pRotated_output_col;
		//manson: we don't treat the original horizontal too long as a special case because
		//we will still convert the original black lines as if they are valid lines. 
		for(c2r_counter=0;c2r_counter<TV_ver_lines;c2r_counter++){
			pTempNew = pOutputcolumnNew;
			pTempOld = pOutputcolumnOld;
			//pCurnInputRow = &final_output_buf_Y[c2r_counter*Final_Output_Buf_Stride_Y];
			pCurnInputRow = &pDecFBPic->pYAddr[c2r_counter*Final_Output_Buf_Stride_Y];
			pCurnInput = pCurnInputRow+current_hor_start_line;
			//row to column convertion
			fraction_counter_r2c = 0;
			for(r2c_counter=0;r2c_counter<old_width;r2c_counter++){
				curn_input = *pCurnInput;
				while(r2c_counter>(fraction_counter_r2c>>15)){ //ready to output
					fraction = fraction_counter_r2c & 0x7fff;
					temp_result = *(pCurnInput-1)*(0x8000-fraction) + curn_input * fraction;
					temp_result = temp_result >> 8;
					*pTempNew = temp_result;
					pTempNew++;
					fraction_counter_r2c = fraction_counter_r2c + width_to_heigh_ratio;
				}
				read_checksumY2 = read_checksumY2 + curn_input;
				pCurnInput++;
			}
			//column to row convertion
			pTempNew = pOutputcolumnNew; //move this pointer back to its beginning
			while(c2r_counter > (fraction_counter_c2r>>15)){ //ready to output
				fraction = fraction_counter_c2r & 0x7fff;
				fraction2 = (0x8000-fraction);
				pCurn_rotate_output = pRotated_output_col; //change013
				pTempNew2 = pTempNew;
				pTempOld2 = pTempOld;
				for(i=0;i<output_rows;i++){
					temp_result = *pTempOld2 * fraction2 + *pTempNew2 * fraction;
					temp_result = temp_result >> 22;
					*pCurn_rotate_output = (uchar)temp_result; //22=15+7
					write_checksumY1 = write_checksumY1+(temp_result&0xff);
					pCurn_rotate_output = pCurn_rotate_output - Final_Output_Buf_Stride_Y; //change011
					pTempOld2++;
					pTempNew2++;
				}
				pRotated_output_col = pRotated_output_col+1;
				output_column_counter ++;
				fraction_counter_c2r = fraction_counter_c2r + height_to_width_ratio;
			}
			//swap column pointers
			pSwap = pOutputcolumnNew;
			pOutputcolumnNew = pOutputcolumnOld;
			pOutputcolumnOld = pSwap;
		}
		//now check the writing
		pRotated_output_row = pFirst_Rotated_output;
		write_checksumY2 = 0;
		for(row_counter=0;row_counter<output_rows;row_counter++){
			pCurn_rotate_output = pRotated_output_row;
			for(col_counter=0;col_counter<output_column_counter;col_counter++){
				write_checksumY2 = write_checksumY2 + *pCurn_rotate_output;
				pCurn_rotate_output++;
			}
			pRotated_output_row = pRotated_output_row-Final_Output_Buf_Stride_Y;
		}
		if((read_checksumY2==read_checksumY1)&&(write_checksumY2==write_checksumY1))
			break;
		else
			data_error_counter++;
	}
	//rotate Cb
	for(try_times=0;try_times<max_try_time;try_times++){
		read_checksumCb2 = 0;
		write_checksumCb1 = 0;
		output_column_counter = 0;
		pOutputcolumnNew = line_buffer_1;
		pOutputcolumnOld = line_buffer_2;
		for(i=0;i<576;i++){
			line_buffer_1[i] = 128<<7;
			line_buffer_2[i] = 128<<7;
		}
		fraction_counter_c2r = 0;
		if(flag_add_black_row!=0){
			pRotated_output_col = &rotated_output_buf_Cb[(last_row>>1)*Final_Output_Buf_Stride_CbCr];
		}
		else{
			pRotated_output_col = &rotated_output_buf_Cb[((TV_ver_lines>>1)-1)*Final_Output_Buf_Stride_CbCr+(first_column>>1)];
		}
		pFirst_Rotated_output = pRotated_output_col;
		for(c2r_counter=0;c2r_counter<(TV_ver_lines>>1);c2r_counter++){
			pTempNew = pOutputcolumnNew;
			pTempOld = pOutputcolumnOld;
			//pCurnInputRow = &final_output_buf_Cb[c2r_counter*Final_Output_Buf_Stride_CbCr];
			pCurnInputRow = &pDecFBPic->pCAddr[c2r_counter*Final_Output_Buf_Stride_CbCr];
			pCurnInput = pCurnInputRow+(current_hor_start_line>>1);
			//row to column convertion
			fraction_counter_r2c = 0;
			for(r2c_counter=0;r2c_counter<(old_width>>1);r2c_counter++){
				curn_input = *pCurnInput;
				while(r2c_counter>(fraction_counter_r2c>>15)){ //ready to output
					fraction = fraction_counter_r2c & 0x7fff;
					temp_result = *(pCurnInput-1)*(0x8000-fraction) + curn_input * fraction;
					temp_result = temp_result >> 8;
					*pTempNew = temp_result;
					pTempNew++;
					fraction_counter_r2c = fraction_counter_r2c + width_to_heigh_ratio;
				}
				read_checksumCb2 = read_checksumCb2 + curn_input;
				pCurnInput++;
			}
			//column to row convertion
			pTempNew = pOutputcolumnNew; //move this pointer back to its beginning
			while(c2r_counter > (fraction_counter_c2r>>15)){ //ready to output
				fraction = fraction_counter_c2r & 0x7fff;
				fraction2 = (0x8000-fraction);
				pCurn_rotate_output = pRotated_output_col; //change013
				pTempNew2 = pTempNew;
				pTempOld2 = pTempOld;
				for(i=0;i<(output_rows>>1);i++){
					temp_result = *pTempOld2 * fraction2 + *pTempNew2 * fraction;
					temp_result = temp_result >> 22;
					*pCurn_rotate_output = (uchar)temp_result; //22=15+7
					write_checksumCb1 = write_checksumCb1+(temp_result&0xff);
					pCurn_rotate_output = pCurn_rotate_output - Final_Output_Buf_Stride_CbCr; //change011
					pTempOld2++;
					pTempNew2++;
				}
				pRotated_output_col = pRotated_output_col+1;
				output_column_counter ++;
				fraction_counter_c2r = fraction_counter_c2r + height_to_width_ratio;
			}
			//swap column pointers
			pSwap = pOutputcolumnNew;
			pOutputcolumnNew = pOutputcolumnOld;
			pOutputcolumnOld = pSwap;
		}
		//now check the writing
		pRotated_output_row = pFirst_Rotated_output;
		write_checksumCb2 = 0;
		for(row_counter=0;row_counter<(output_rows>>1);row_counter++){
			pCurn_rotate_output = pRotated_output_row;
			for(col_counter=0;col_counter<output_column_counter;col_counter++){
				write_checksumCb2 = write_checksumCb2 + *pCurn_rotate_output;
				pCurn_rotate_output++;
			}
			pRotated_output_row = pRotated_output_row-Final_Output_Buf_Stride_CbCr;
		}
		if((read_checksumCb2==read_checksumCb1)&&(write_checksumCb2==write_checksumCb1))
			break;
		else
			data_error_counter++;
	}
	//rotate Cr
	for(try_times=0;try_times<max_try_time;try_times++){
		read_checksumCr2 = 0;
		write_checksumCr1 = 0;
		output_column_counter = 0;
		pOutputcolumnNew = line_buffer_1;
		pOutputcolumnOld = line_buffer_2;
		for(i=0;i<576;i++){
			line_buffer_1[i] = 128<<7;
			line_buffer_2[i] = 128<<7;
		}
		fraction_counter_c2r = 0;
		if(flag_add_black_row!=0){
			pRotated_output_col = &rotated_output_buf_Cr[(last_row>>1)*Final_Output_Buf_Stride_CbCr];
		}
		else{
			pRotated_output_col = &rotated_output_buf_Cr[((TV_ver_lines>>1)-1)*Final_Output_Buf_Stride_CbCr+(first_column>>1)];
		}
		pFirst_Rotated_output = pRotated_output_col;
		for(c2r_counter=0;c2r_counter<(TV_ver_lines>>1);c2r_counter++){
			pTempNew = pOutputcolumnNew;
			pTempOld = pOutputcolumnOld;
			//pCurnInputRow = &final_output_buf_Cr[c2r_counter*Final_Output_Buf_Stride_CbCr];
			pCurnInputRow = &pDecFBPic->pCAddr[c2r_counter*Final_Output_Buf_Stride_CbCr];
			pCurnInput = pCurnInputRow+(current_hor_start_line>>1);
			//row to column convertion
			fraction_counter_r2c = 0;
			for(r2c_counter=0;r2c_counter<(old_width>>1);r2c_counter++){
				curn_input = *pCurnInput;
				while(r2c_counter>(fraction_counter_r2c>>15)){ //ready to output
					fraction = fraction_counter_r2c & 0x7fff;
					temp_result = *(pCurnInput-1)*(0x8000-fraction) + curn_input * fraction;
					temp_result = temp_result >> 8;
					*pTempNew = temp_result;
					pTempNew++;
					fraction_counter_r2c = fraction_counter_r2c + width_to_heigh_ratio;
				}
				read_checksumCr2 = read_checksumCr2 + curn_input;
				pCurnInput++;
			}
			//column to row convertion
			pTempNew = pOutputcolumnNew; //move this pointer back to its beginning
			while(c2r_counter > (fraction_counter_c2r>>15)){ //ready to output
				fraction = fraction_counter_c2r & 0x7fff;
				fraction2 = (0x8000-fraction);
				pCurn_rotate_output = pRotated_output_col; //change013
				pTempNew2 = pTempNew;
				pTempOld2 = pTempOld;
				for(i=0;i<(output_rows>>1);i++){
					temp_result = *pTempOld2 * fraction2 + *pTempNew2 * fraction;
					temp_result = temp_result >> 22;
					*pCurn_rotate_output = (uchar)temp_result; //22=15+7
					write_checksumCr1 = write_checksumCr1+(temp_result&0xff);
					pCurn_rotate_output = pCurn_rotate_output - Final_Output_Buf_Stride_CbCr; //change011
					pTempOld2++;
					pTempNew2++;
				}
				pRotated_output_col = pRotated_output_col+1;
				output_column_counter ++;
				fraction_counter_c2r = fraction_counter_c2r + height_to_width_ratio;
			}
			//swap column pointers
			pSwap = pOutputcolumnNew;
			pOutputcolumnNew = pOutputcolumnOld;
			pOutputcolumnOld = pSwap;
		}
		//now check the writing
		pRotated_output_row = pFirst_Rotated_output;
		write_checksumCr2 = 0;
		for(row_counter=0;row_counter<(output_rows>>1);row_counter++){
			pCurn_rotate_output = pRotated_output_row;
			for(col_counter=0;col_counter<output_column_counter;col_counter++){
				write_checksumCr2 = write_checksumCr2 + *pCurn_rotate_output;
				pCurn_rotate_output++;
			}
			pRotated_output_row = pRotated_output_row-Final_Output_Buf_Stride_CbCr;
		}
		if((read_checksumCr2==read_checksumCr1)&&(write_checksumCr2==write_checksumCr1))
			break;
		else
			data_error_counter++;
	}
	//add black line at the beginning and the end to hide interpolation problems
/*
	for(hor_counter=0;hor_counter<720;hor_counter++)
		rotated_output_buf_Y[hor_counter] = 0;
	for(hor_counter=0;hor_counter<(720>>1);hor_counter++)
		rotated_output_buf_Cb[hor_counter] = 128;
	for(hor_counter=0;hor_counter<(720>>1);hor_counter++)
		rotated_output_buf_Cr[hor_counter] = 128;
	for(hor_counter=0;hor_counter<720;hor_counter++)
		rotated_output_buf_Y[Final_Output_Buf_Stride_Y*(TV_ver_lines-1)+hor_counter] = 0;
	for(hor_counter=0;hor_counter<(720>>1);hor_counter++)
		rotated_output_buf_Cb[Final_Output_Buf_Stride_CbCr*((TV_ver_lines>>1)-1)+hor_counter] = 128;
	for(hor_counter=0;hor_counter<(720>>1);hor_counter++)
		rotated_output_buf_Cr[Final_Output_Buf_Stride_CbCr*((TV_ver_lines>>1)-1)+hor_counter] = 128;
	for(ver_counter=0;ver_counter<TV_ver_lines;ver_counter++)
		rotated_output_buf_Y[Final_Output_Buf_Stride_Y*ver_counter+719] = 0;
	for(ver_counter=0;ver_counter<(TV_ver_lines>>1);ver_counter++)
		rotated_output_buf_Cb[Final_Output_Buf_Stride_CbCr*ver_counter+359] = 128;
	for(ver_counter=0;ver_counter<(TV_ver_lines>>1);ver_counter++)
		rotated_output_buf_Cr[Final_Output_Buf_Stride_CbCr*ver_counter+359] = 128;
*/
#endif
}

//////////////////////////////////////////////////////////
//manson: rotate the picture store in one frame buffer by 270 degree, clockwise
//Here implement a quick solution, rotate by 90 degree and then turns upside down
void Rotate90()
{
#if 0 //john
	int ver_counter;
	int hor_counter;
	int row_index1;
	int row_index2;

	Rotate270();

	for(ver_counter=0;ver_counter<(TV_ver_lines>>1);ver_counter++){
		row_index1 = ver_counter * Final_Output_Buf_Stride_Y;
		row_index2 = (TV_ver_lines-ver_counter-1) * Final_Output_Buf_Stride_Y;
		for(hor_counter=0;hor_counter<720;hor_counter++){
			line_buffer_1[hor_counter] = rotated_output_buf_Y[row_index1+(719-hor_counter)];
			line_buffer_2[hor_counter] = rotated_output_buf_Y[row_index2+(719-hor_counter)];
		}
		for(hor_counter=0;hor_counter<720;hor_counter++){
			rotated_output_buf_Y[row_index1+hor_counter] = line_buffer_2[hor_counter];
			rotated_output_buf_Y[row_index2+hor_counter] = line_buffer_1[hor_counter];
		}
	}
	for(ver_counter=0;ver_counter<(TV_ver_lines>>2);ver_counter++){
		row_index1 = ver_counter * Final_Output_Buf_Stride_CbCr;
		row_index2 = ((TV_ver_lines>>1)-ver_counter-1) * Final_Output_Buf_Stride_CbCr;
		for(hor_counter=0;hor_counter<360;hor_counter++){
			line_buffer_1[hor_counter] = rotated_output_buf_Cb[row_index1+(359-hor_counter)];
			line_buffer_2[hor_counter] = rotated_output_buf_Cb[row_index2+(359-hor_counter)];
		}
		for(hor_counter=0;hor_counter<360;hor_counter++){
			rotated_output_buf_Cb[row_index1+hor_counter] = line_buffer_2[hor_counter];
			rotated_output_buf_Cb[row_index2+hor_counter] = line_buffer_1[hor_counter];
		}
		for(hor_counter=0;hor_counter<360;hor_counter++){
			line_buffer_1[hor_counter] = rotated_output_buf_Cr[row_index1+(359-hor_counter)];
			line_buffer_2[hor_counter] = rotated_output_buf_Cr[row_index2+(359-hor_counter)];
		}
		for(hor_counter=0;hor_counter<360;hor_counter++){
			rotated_output_buf_Cr[row_index1+hor_counter] = line_buffer_2[hor_counter];
			rotated_output_buf_Cr[row_index2+hor_counter] = line_buffer_1[hor_counter];
		}
	}
#endif
}

void Rotate180()
{
#if 0 //john
	int ver_counter;
	int hor_counter;
	int row_index1;
	int row_index2;

	for(ver_counter=0;ver_counter<(TV_ver_lines>>1);ver_counter++){
		row_index1 = ver_counter * Final_Output_Buf_Stride_Y;
		row_index2 = (TV_ver_lines-ver_counter-1) * Final_Output_Buf_Stride_Y;
		for(hor_counter=0;hor_counter<720;hor_counter++){
			rotated_output_buf_Y[row_index1+hor_counter] = final_output_buf_Y[row_index2+(719-hor_counter)];
			rotated_output_buf_Y[row_index2+hor_counter] = final_output_buf_Y[row_index1+(719-hor_counter)];
		}
	}
	for(ver_counter=0;ver_counter<(TV_ver_lines>>2);ver_counter++){
		row_index1 = ver_counter * Final_Output_Buf_Stride_CbCr;
		row_index2 = ((TV_ver_lines>>1)-ver_counter-1) * Final_Output_Buf_Stride_CbCr;
		for(hor_counter=0;hor_counter<360;hor_counter++){
			rotated_output_buf_Cb[row_index1+hor_counter] = final_output_buf_C[row_index2+(359-hor_counter)];
			rotated_output_buf_Cb[row_index2+hor_counter] = final_output_buf_C[row_index1+(359-hor_counter)];

			rotated_output_buf_Cr[row_index1+hor_counter] = final_output_buf_C[row_index2+(359-hor_counter)];
			rotated_output_buf_Cr[row_index2+hor_counter] = final_output_buf_C[row_index1+(359-hor_counter)];
		}
	}
	mem_memcpy(final_output_buf_Y, rotated_output_buf_Y, 1024*576);
	mem_memcpy(final_output_buf_C, rotated_output_buf_Cb, 512*288);

#endif
}



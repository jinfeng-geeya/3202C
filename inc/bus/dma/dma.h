#ifndef  _DMA_H_
#define  _DMA_H_

//DMA channel number
enum DMA_CHANNEL_NUM{
	DMA_CH0 = 0, 
	DMA_CH1,
	DMA_CH2,
	DMA_CH3,
	DMA_CH4,
	DMA_CH5,
	DMA_CH6,
	DMA_CH7, 
	DMA_CHAANY=8, //to open any channel available
	DMA_INVALID_CHA=0XFF
};

/*channel flags to open a channel */
#define DMA_SHARE_BY_MULTI_TASK	0X00000001 
									/*the channel can be shared by multiple tasks */
//the following channel flags are only valid for non-sharable channels 
#define DMA_DETECT_START_CODE 	0X00000002
									/* the channel can detect the MPEG start codes */
#define DMA_BITRATE_HIGH			0X00000010
									/* bitrate expectation is high*/
#define DMA_BITRATE_NORMAL		0X00000020
									/* bitrate expectation is normal*/
#define DMA_BITRATE_LOW			0X00000040
									/* bitrate expectation is low*/

//channel start code attribute
struct startcode_attr_t
{
	UINT8  	rng1;//range 1
	UINT8  	rng2;//range 2
	UINT8  	inside_rng; 
			/*if TRUE,  the starts codes to detect is within [rng1, rng2], 
	   		otherwise,  the starts codes to detect is within [0x00, rng1) or (rng2, 0xff] */
};

//a start code record
struct start_code_t
{
	UINT8 	val;		// value of the start code
	UINT32	addr;	// physical destination address of the start code
};

//transfer request flags
#define DMA_SYNC_TRANSFER			0X01  //this transfer need to be synchronized
#define DMA_FORBIDDEN_WRITE			0x20
#define DMA_CONTINUOUS_NODE			0x40


#define DMA_MAX_XFER_SIZE		0x16//0XFFFF //maximum trnsfer size in bytes


//transfer id to wait until all transfers have completed. 
//#define DMA_XFRID_WAITALL 	0xFF000000

//invalid transfer id
#define DMA_INVALID_XFR_ID 0XFFFFFFFF 

//wait mode
#define  DMA_WAIT_MODE_DEFALT 0X00
#define DMA_WAIT_MODE_POLLING 0X01

#define DMA_LIST_PRIORITY_HIGH	0x01
#define DMA_LIST_PRIORITY_LOW	0x02

RET_CODE 	dma_init();
UINT8 		dma_open(UINT8 chaNum, UINT32 flags, struct startcode_attr_t  *sc_attr);
void 		dma_close(UINT8 chNum);
UINT32 		dma_copy(UINT8 chNum,void *src, void *dst, UINT16  byteCnt, UINT8 flags );


void 		dma_wait(UINT32 id, UINT8 mode);
RET_CODE 	dma_get_start_code( UINT8 chNum, struct start_code_t *start_code);

#endif



#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libdbc/lib_dbc.h>

#ifndef DISABLE_OTA

#define DBC_PRINTF	PRINTF

BOOL dsi_message(UINT8* data,UINT16 len,UINT32 OUI,UINT16 hw_model,UINT16 hw_version,UINT16 sw_model,UINT16 sw_version,UINT16* new_sw_version,struct DCGroupInfo* group)
{
	UINT32 i,j,k;
	UINT8 byte1,byte2,byte3,byte4;
	
	UINT32 transaction_id;
	UINT8 adaptation_len;
	UINT16 message_len;
	UINT16 private_data_len;
	UINT16 num_of_groups;
	UINT32 group_id;
	UINT32 group_size;
	UINT16 compatibility_descriptor_len;
	UINT16 descriptor_count;
	UINT8 descriptor_type;
	UINT8 descriptor_len;
	UINT8 specifier_type;
	UINT32 specifier_data;
	UINT16 model;
	UINT16 version;

	UINT16 group_info_len;
	UINT8 descriptor_tag;
	
	
	BOOL hardware_fit = FALSE;
	BOOL software_fit = FALSE;

	DBC_PRINTF("DSI message!\n");
/* dsmccMessageHeader */
	data++;
	data++;
	data++;
	data++;
	byte1 = *data++;
	byte2 = *data++;
	byte3 = *data++;
	byte4 = *data++;	
	transaction_id = (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
	data++;
	adaptation_len = *data++;
	byte1 = *data++;
	byte2 = *data++;
	message_len = (byte1<<8)+byte2;

	DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d \n",len,adaptation_len,message_len);
	
	data += adaptation_len;	
/*~ dsmccMessageHeader */

	//serverid
	data+=20;
	//compatibilityDescriptor()  : 0x0000
	byte1 = *data++;
	byte2 = *data++;
	compatibility_descriptor_len = (byte1<<8)+byte2;
	data += compatibility_descriptor_len;
	
	byte1 = *data++;
	byte2 = *data++;
	private_data_len = (byte1<<8)+byte2;

	byte1 = *data++;
	byte2 = *data++;
	num_of_groups = (byte1<<8)+byte2;

	for(i=0;i<num_of_groups;i++)
	{
		hardware_fit = FALSE;
		software_fit = FALSE;
		byte1 = *data++;
		byte2 = *data++;
		byte3 = *data++;
		byte4 = *data++;	
		group_id =  (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;	
		byte1 = *data++;
		byte2 = *data++;
		byte3 = *data++;
		byte4 = *data++;	
		group_size =  (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
		DBC_PRINTF("group_id = 0x%x,group_size = %d\n",group_id,group_size);
		/*GroupCompatibility*/
		byte1 = *data++;
		byte2 = *data++;
		compatibility_descriptor_len = (byte1<<8)+byte2;
		if(compatibility_descriptor_len<2)
		{
			data+=compatibility_descriptor_len;
		}
		else
		{
			compatibility_descriptor_len -= 2;
			byte1 = *data++;
			byte2 = *data++;
			descriptor_count = (byte1<<8)+byte2;		
			for(j=0;j<descriptor_count;j++)
			{
				descriptor_type = *data++;
				descriptor_len = *data++;
				if(compatibility_descriptor_len<descriptor_len+2)
				{
					data--;
					data--;

					data += compatibility_descriptor_len;
					compatibility_descriptor_len = 0;
					break;
				}
				else
				{
					specifier_type = *data++;
					byte1 = *data++;
					byte2 = *data++;
					byte3 = *data++;
					specifier_data =  (byte1<<16) +(byte2<<8)+byte3;	
					byte1 = *data++;
					byte2 = *data++;
					model = (byte1<<8)+byte2;			
					byte1 = *data++;
					byte2 = *data++;
					version = (byte1<<8)+byte2;	

					data += descriptor_len-8;
					if(descriptor_type == 0x01 && specifier_type == 0x01 && specifier_data == OUI  && model == hw_model && version == hw_version)
					{
						hardware_fit = TRUE;
					}
					//if(descriptor_type == 0x02 && specifier_type == 0x01 && specifier_data == OUI  && model == sw_model && version >= sw_version)
					if(descriptor_type == 0x02 && specifier_type == 0x01 && specifier_data == OUI  && model == sw_model)
					{	
						*new_sw_version = version;
						software_fit = TRUE;	
					}
					
					compatibility_descriptor_len -= (descriptor_len+2);				
					
				}
			}
			data+= compatibility_descriptor_len;

		}
		group->group_id = group_id;
		group->group_size = group_size;

		/*~GroupCompatibility*/
		
		byte1 = *data++;
		byte2 = *data++;
		group_info_len = (byte1<<8)+byte2;	
#ifdef STAR_OTA_GROUP_INFO_BYTE
		byte1 = *data++;
		byte2 = *data++;
		byte3 = *data++;
		specifier_data =  (byte1<<16) +(byte2<<8)+byte3;	
		if(specifier_data == OUI && group_info_len>=0x14)
		{
			group->OUI = OUI;
			byte1 = *data++;
			byte2 = *data++;
			byte3 = *data++;
			byte4 = *data++;			
			specifier_data = (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
			group->global_hw_ver = specifier_data;

			byte1 = *data++;
			byte2 = *data++;
			byte3 = *data++;
			byte4 = *data++;			
			specifier_data = (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
			group->global_sw_ver = specifier_data;

			byte1 = *data++;
			byte2 = *data++;
			byte3 = *data++;
			byte4 = *data++;			
			specifier_data = (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
			group->stbid_start = specifier_data;

			byte1 = *data++;
			byte2 = *data++;
			byte3 = *data++;
			byte4 = *data++;			
			specifier_data = (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
			group->stbid_end = specifier_data;

			data += (group_info_len - 0x13);

		}
		else
			data += (group_info_len - 3);
		
#else
		while(group_info_len>0)
		{
			descriptor_tag = *data++;
			descriptor_len = *data++;
			if(group_info_len<descriptor_len+2)
			{
				return FALSE;//ASSERT(0);
				data--;
				data--;
				data += group_info_len;
				break;
			}
			else
			{
				switch(descriptor_tag)
				{
					case 0x01://type
						{
							DBC_PRINTF("--------type Desc. \n");
							data+=descriptor_len;
							break; 
						}
					case 0x02://name		
						{
							DBC_PRINTF("--------name Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x03://info		
						{
							DBC_PRINTF("--------info Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x06://location
						{
							DBC_PRINTF("--------location Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x07://est_download_time	
						{
							DBC_PRINTF("--------est_download_time Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x08://group_link
						{
							DBC_PRINTF("--------group_link Desc. \n");	
							data+=descriptor_len;
							break;
						}
					case 0x0a:
						{
							DBC_PRINTF("--------0x0a Desc. \n");	
							data+=descriptor_len;
							break;
						}
					case 0x04:
					case 0x05:					
					case 0x09:	
					default:
						{
							DBC_PRINTF("--------other Desc. \n");
							data+=descriptor_len;
							break;
						}
				}	
				group_info_len -= (descriptor_len+2);
			}
		}
#endif

		//BEGIN GROUPINFOINDICATION_102_006  //inside
		byte1 = *data++;
		byte2 = *data++;
		private_data_len = (byte1<<8)+byte2;
		data+=private_data_len;
		//END GROUPINFOINDICATION_102_006  //inside
		
		if(hardware_fit == TRUE && software_fit == TRUE)
		{
			DBC_PRINTF("Group  0x%x Fit !\n",group->group_id);
			return TRUE;
		}

	}
	return FALSE;	

/* If any usage of following	*/
#if 0
// GROUPINFOINDICATION_301_192	//outside						
	byte1 = *data++;
	byte2 = *data++;
	private_data_len = (byte1<<8)+byte2;

	data+=private_data_len;
#endif
	
}


BOOL dii_message(UINT8* data,UINT16 len,UINT32 group_id,UINT8* data_addr,struct DCModuleInfo* module,UINT8* module_num,UINT16* blocksize)
{
	UINT32 i,j,k,module_index;
	UINT8 byte1,byte2,byte3,byte4;
	
	UINT32 transaction_id;
	UINT8 adaptation_len;
	UINT16 message_len;
	
	UINT32 download_id;
	UINT16 compatibility_descriptor_len;
	UINT16 num_of_modules;
	UINT16 module_id;
	UINT32 module_size;
	UINT8 module_version;
	UINT8 module_info_len;
	UINT8 descriptor_tag;
	UINT8 descriptor_len;
	UINT8 position;
	UINT16 next_module_id;

	UINT16 private_data_len;

	DBC_PRINTF("DDI message!\n");	
/* dsmccMessageHeader */
	data++;
	data++;
	data++;
	data++;
	byte1 = *data++;
	byte2 = *data++;
	byte3 = *data++;
	byte4 = *data++;	
	transaction_id = (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
	if(transaction_id!= group_id)
	{
		DBC_PRINTF("DII transaction_id = %8x \n",transaction_id);
		return FALSE;
	}
	data++;
	adaptation_len = *data++;
	byte1 = *data++;
	byte2 = *data++;
	message_len = (byte1<<8)+byte2;

	DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d \n",len,adaptation_len,message_len);
	
	data+=adaptation_len;
/*~ dsmccMessageHeader */

	byte1 = *data++;
	byte2 = *data++;
	byte3 = *data++;
	byte4 = *data++;	
	download_id = (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
	
	byte1 = *data++;
	byte2 = *data++;	
	*blocksize = (byte1<<8)+byte2;	
	
	data++;//windowsize
	data++;//ackperiod
	
	data++;
	data++;
	data++;
	data++;//tCDownloadWindow
	data++;
	data++;
	data++;
	data++;//tCDownloadScenario

	//compatibilityDescriptor()  : 0x0000
	byte1 = *data++;
	byte2 = *data++;
	compatibility_descriptor_len = (byte1<<8)+byte2;
	data += compatibility_descriptor_len;
	

	byte1 = *data++;
	byte2 = *data++;
	
	num_of_modules = (byte1<<8)+byte2;
	*module_num  = num_of_modules;

	for(module_index=0;module_index<num_of_modules;module_index++)
	{
		byte1 = *data++;
		byte2 = *data++;
		module_id = (byte1<<8)+byte2;	
		byte1 = *data++;
		byte2 = *data++;
		byte3 = *data++;
		byte4 = *data++;	
		module_size =  (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
		module_version = *data++;

		if(module_index == 0)
		{
			module[module_index].module_buf_pointer = data_addr;
		}
		else
		{
			module[module_index].module_buf_pointer = module[module_index-1].module_buf_pointer + module[module_index-1].module_buf_size;
		}
		module[module_index].module_id = module_id;
		module[module_index].module_buf_size = module_size;
		if(module_size%(*blocksize))
			module[module_index].block_num = module_size/(*blocksize)+1;
		else
			module[module_index].block_num = module_size/(*blocksize);
		module[module_index].module_linked = 0;
		module[module_index].module_first = 0;
		module[module_index].module_last = 0;
		module[module_index].next_block_num = 0;
		for(i=0;i<BIT_MAP_NUM;i++)
			module[module_index].block_received[i] = 0xffffffff;
		for(i=0;i<module[module_index].block_num;i++)		
			module[module_index].block_received[i/32] &= ~(1<<(i%32));	
		module[module_index].module_download_finish = 0;
		
		module_info_len = *data++;
		while(module_info_len>0)
		{
			descriptor_tag = *data++;
			descriptor_len = *data++;
			if(module_info_len<descriptor_len+2)
			{
				return FALSE;//ASSERT(0);
				data--;
				data--;
				data+=module_info_len;
				break;
			}
			else
			{
				switch(descriptor_tag)
				{
					case 0x04://module_link
						DBC_PRINTF("--------module_link Desc. \n");
						if(descriptor_len <3)
							ASSERT(0);
						position = *data++;
						byte1 = *data++;
						byte2 = *data++;
						next_module_id = (byte1<<8)+byte2;		
						module[module_index].module_linked = 1;
						if(position == 0x00)
						{
							module[module_index].module_first = 1;
							module[module_index].next_module_id = next_module_id;
						}
						else if(position == 0x01)
						{
							module[module_index].next_module_id = next_module_id;				
						}
						else if(position == 0x02)
						{
							module[module_index].module_last = 1;
						}						
						data += descriptor_len-3;
						break;
						
					case 0x01://type
						{
							DBC_PRINTF("--------type Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x02://name		
						{
							DBC_PRINTF("--------name Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x03://info		
						{
							DBC_PRINTF("--------info Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x05://CRC32
						{
							DBC_PRINTF("--------CRC32 Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x06://location
						{
							DBC_PRINTF("--------location Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x07://est_download_time	
						{
							DBC_PRINTF("--------est_download_time Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x09://compressed_module
						{
							DBC_PRINTF("--------compressed_module Desc. \n");
							data+=descriptor_len;
							break;
						}
					case 0x08:
					case 0x0a:
					default:
						{
							DBC_PRINTF("--------Other Desc. \n");
							data+=descriptor_len;
							break;
						}		
				}	
				module_info_len -= (descriptor_len+2);
			}
		}

	}
	
	byte1 = *data++;
	byte2 = *data++;
	private_data_len = (byte1<<8)+byte2;
	
	data+=private_data_len;	

	return TRUE;

}
BOOL ddb_data(UINT8* data,UINT16 len,struct DCModuleInfo* module,UINT16 blocksize,UINT8 blk_id)
{
	UINT32 i,j,k,module_index;
	UINT8 byte1,byte2,byte3,byte4;
	
	UINT32 download_id;
	UINT8 adaptation_len;
	UINT16 message_len;	

	UINT16 module_id;
	UINT8 module_version;
	UINT32 module_size;
	UINT16 block_num;
	UINT16 block_len;
	UINT8* module_pointer;
	UINT8* block_pointer;
	
	BOOL module_find;

	DBC_PRINTF("DDB data!\n");	
/* dsmccDownloadDataHeader */
	data++;
	data++;
	data++;
	data++;
	byte1 = *data++;
	byte2 = *data++;
	byte3 = *data++;
	byte4 = *data++;	
	download_id = (byte1<<24) +(byte2<<16)+(byte3<<8)+byte4;
	data++;
	adaptation_len = *data++;
	byte1 = *data++;
	byte2 = *data++;
	message_len = (byte1<<8)+byte2;

	//DBC_PRINTF("section_len = %d,adaptation_len = %d,message_len = %d \n",len,adaptation_len,message_len);
	
	data+=adaptation_len;
/*~ dsmccDownloadDataHeader */

	byte1 = *data++;
	byte2 = *data++;
	module_id = (byte1<<8)+byte2;
	module_version = *data++;
	data++;//reserved
	
	module_find = FALSE;

	if(module->module_id != module_id)
	{
		return FALSE;
	}

	module_size = module->module_buf_size;
	module_pointer = module->module_buf_pointer;

	byte1 = *data++;
	byte2 = *data++;
	block_num = (byte1<<8)+byte2;	
	DBC_PRINTF("block_num = %d \n",block_num);
	if((UINT8)block_num != blk_id)
	{
		DBC_PRINTF("ERROR : (UINT8)block_num != blk_id!\n");
		return FALSE;
	}		
	if(block_num > module->block_num-1)
	{
		DBC_PRINTF("ERROR : block_num > module->block_num-1!\n");
		return FALSE;
	}	
	/*
	if(block_num != module->next_block_num)
	{
		return FALSE;
	}
	else
	{
		module->next_block_num++;
	}
	*/
	
	if((UINT32)(block_num*blocksize) >= module_size)
	{
		return FALSE;//ASSERT(0);//return;		
	}
	else if(module->block_received[block_num/32]&(1<<(block_num%32)))
	{
		DBC_PRINTF("block exist\n");
		return FALSE;
	}
	else if((module_size-block_num*blocksize)>blocksize)
	{
		block_len = blocksize;
	}
	else
	{
		block_len = module_size-block_num*blocksize;
		//module->module_download_finish = 1;
	}
	
	block_pointer = module_pointer + block_num*blocksize;
	
	if(message_len-6 < block_len)
	{
		return FALSE;//ASSERT(0);//return;
	}
	MEMCPY(block_pointer,data,block_len);

	module->block_received[block_num/32] |= (1<<(block_num%32));

	/* check all module download info */
	module->module_download_finish = 1;
	for(i=0;i<BIT_MAP_NUM;i++)
		if(module->block_received[i] != 0xffffffff)
			module->module_download_finish = 0;

	return TRUE;

}

#endif


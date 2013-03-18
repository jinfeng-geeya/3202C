#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>

#include <api/libsi/si_module.h>
#include <api/libsi/si_nit.h>
#include <api/libsi/si_utility.h>
#include <api/libsi/si_desc_cab_delivery.h>

#define SCD_DEBUG_LEVEL			0
#if (SCD_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SCD_PRINTF			libc_printf
#else
#define SCD_PRINTF(...)			do{}while(0)
#endif


INT32 si_on_cab_delivery_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
	struct nit_section_info *n_info = (struct nit_section_info *)priv;
	struct cable_delivery_system_descriptor *cable = (struct cable_delivery_system_descriptor *)(data-2);
	TP_INFO *tp_info;
       UINT32 frq,sym;
    
	if (n_info->xp_nr==PSI_MODULE_MAX_TP_NUMBER)
		return SI_SUCCESS;

       frq = bcd2integer(cable->frequency, 8, 0)/100;
       sym = bcd2integer(cable->symbol_rate, 7, 0)/10;
       /*check if tp parameter correct*/
       if((frq<4800)||(frq>85900)||(sym<1000)||(sym>7000)
        ||(cable->modulation<=0x0)||(cable->modulation>0x05))
           return SI_SUCCESS;
            
	tp_info	= &n_info->xp[n_info->xp_nr++];
	tp_info->c_info.tsid = n_info->tsid;
	tp_info->c_info.onid = n_info->onid;
	
	tp_info->c_info.frequency = frq;
	tp_info->c_info.symbol_rate = (UINT16)sym;
	tp_info->c_info.modulation = (UINT16)(cable->modulation + 3);

	//tp_info->c_info.FEC_outer = cable->FEC_outer;
	tp_info->c_info.FEC_inner = cable->FEC_inner;

        SCD_PRINTF("%s(): freq[%d], symbol_rate[%d], qam[%d]\n",__FUNCTION__,
            tp_info->c_info.frequency, tp_info->c_info.symbol_rate, tp_info->c_info.modulation);
	
	return SI_SUCCESS;
}


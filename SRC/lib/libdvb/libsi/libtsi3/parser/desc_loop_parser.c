#include <types.h>
#include <sys_config.h>

#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>

#include <api/libsi/desc_loop_parser.h>

struct descriptor {
	UINT8 tag;
	UINT8 len;
	UINT8 data[0];
}__attribute__((packed));

INT32 desc_loop_parser(UINT8 *data, INT32 len, struct desc_table *info,
	INT32 nr, UINT32 *stat, void *priv)
{
	INT16 i, l, h, m;
	INT32 desc_len, ret;
	struct descriptor *desc;
	struct desc_table *current;
	for(i=0; i<len; i += sizeof(struct descriptor)+desc_len) {
		desc = (struct descriptor *)(data+i);
		desc_len = desc->len;
		l = -1;
		h = nr;
		do{
			m = (l+h)>>1;
			current = info+m;
			if (current->tag>desc->tag) {
				h = m;
			} else if (current->tag<desc->tag) {
				l = m;
			} else {
				if (stat) {
					*stat |= (1<<current->bit);
				}
				if (current->parser) {
					ret = current->parser(desc->tag,
						desc->len, desc->data, priv);
					if (ret != SI_SUCCESS)
						return ret;
				}
				break;
			}
		} while (h-l>1);
	}
	return SI_SUCCESS;
}

INT32 si_descriptor_parser(UINT8 *data, INT32 length, struct descriptor_info *info, INT32 nr)
{
	INT32 pos, i, k, ret;
	struct descriptor *desc;
	for(pos = 0; pos<length; pos+= desc->len+sizeof(struct descriptor)) {
		desc = (struct descriptor *)(data+pos);
		for(k=0; k<nr; k++) {
			if ((desc->tag == info[k].descriptor_tag)
				&&(info[k].mask_len<=desc->len)){
				for(i=0; i<info[k].mask_len; i++) {
					if ((desc->data[i]&info[k].mask[i]) != (info[k].value[i]&info[k].mask[i]))
						break;
				}
				if (i==info[k].mask_len) {
					ret = info[k].on_descriptor(desc->tag, 
						desc->len, desc->data, info->priv);
					if (ret != SI_SUCCESS) 
						return ret;
					break; /* k */
				} 
			}
		}
	}
	return SI_SUCCESS;
}



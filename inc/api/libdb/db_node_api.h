#ifndef __DB_NODE_API_H__
#define __DB_NODE_API_H__

INT32 DB_regist_packer_unpacker(node_pack_t packer, node_unpack_t unpacker);
INT32 DB_set_update_buf(UINT8 * buf);
//INT32 DB_create_view(UINT8 type, DB_VIEW *v_attr,db_filter_t filter, UINT16 filter_mode, UINT32 filter_param);
INT32 DB_create_view(UINT8 type, DB_VIEW *v_attr,db_filter_t filter, db_filter_pip pip_filter, UINT16 filter_mode, UINT32 filter_param);
INT32 DB_update_operation();
INT32 DB_clear_cmd_buf();

INT32 DB_get_node_by_pos_from_table(DB_TABLE *table, UINT16 pos,UINT32 *node_id, UINT32 *node_addr);
INT32 DB_get_node_by_id_from_table(DB_TABLE *table, UINT32 node_id,UINT32 *node_addr);
INT32 DB_get_node_by_pos(DB_VIEW *v_attr, UINT16 pos, UINT32 *node_id, UINT32 *node_addr );
INT32 DB_get_node_by_id(DB_VIEW *v_attr, UINT32 id, UINT32 *node_addr);
void DB_get_node_id_addr_by_pos(UINT8 view_dismatch,DB_TABLE *table, DB_VIEW *view, UINT16 pos,UINT32 *id, UINT32 *addr);

//INT32 DB_read_data_from_flash(UINT32 addr, UINT32 len, UINT8 * data);
INT32 DB_read_node(UINT32 node_id, UINT32 addr, UINT8 *node,UINT32 len);

INT32 DB_add_node_to_table(DB_TABLE *table, UINT32 node_id, UINT8 *node, UINT32 node_len);

INT32 DB_pre_add_node(DB_VIEW *v_attr, UINT8 n_type, UINT32 parent_id, UINT32 node_len, UINT32 *node_id);
INT32 DB_add_node(DB_VIEW *v_attr, UINT32 node_id,UINT8 *node, UINT32 node_len);

INT32 DB_modify_node_by_id(DB_VIEW *v_attr, UINT32 id, UINT8 *node, UINT32 node_len);
INT32 DB_del_node_by_pos(DB_VIEW *v_attr, UINT16 pos);
INT32 DB_del_node_by_id(DB_VIEW *v_attr, UINT32 id);
INT32 DB_del_node_by_parent_id(DB_VIEW *v_attr, UINT8 parent_type, UINT32 parent_id);
INT32 DB_move_node(DB_VIEW * v_attr, UINT16 src_pos, UINT16 dest_pos);


UINT16 DB_get_node_pos(DB_VIEW *v_attr, UINT32 id);
UINT16  DB_get_node_num(DB_VIEW *v_attr, db_filter_t filter, UINT16 filter_mode,UINT32 filter_param);
INT32 DB_init(UINT32 db_base_addr, UINT32 db_length);
UINT8 DB_get_cur_view_type();
INT32 DB_clear_db();

#endif



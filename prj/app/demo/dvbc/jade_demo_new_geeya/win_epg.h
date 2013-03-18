struct winepginfo
{
	date_time start;
	date_time end;
	UINT16 event_idx;
	UINT8 pos;
};
struct win_epg_item_info
{
	UINT8 count;
	struct winepginfo *epg_info;
};

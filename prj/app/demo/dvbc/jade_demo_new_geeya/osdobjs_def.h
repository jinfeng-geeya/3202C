#ifndef _OSDOBJS_DEF_H_
#define _OSDOBJS_DEF_H_


#define DEF_TEXTFIELD(txt,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,strID,str)   \
TEXT_FIELD txt = \
{   \
    {OT_TEXTFIELD, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,strID,str \
};

#define DEF_BITMAP(bmp,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,iconID)   \
BITMAP bmp = \
{   \
    {OT_BITMAP, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,iconID \
};

#define DEF_CONTAINER(con,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,chlst,focusID,allHilite)   \
CONTAINER con = \
{   \
    {OT_CONTAINER, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    (POBJECT_HEAD)chlst,focusID,allHilite \
};


#define DEF_MULTITEXT(mtxt,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,count,rl,rt,rw,rh,sb,txttbl)   \
MULTI_TEXT mtxt = \
{   \
    {OT_MULTITEXT, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,count,0,{rl,rt,rw,rh},sb,txttbl \
};


#define DEF_OBJECTLIST(ol,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,field,sb,mark,style,dep,cnt,selarray)   \
OBJLIST ol = \
{   \
    {OT_OBJLIST, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    (POBJECT_HEAD*)field,sb,style,dep,cnt,0,0,0,0, selarray,mark\
};

#define DEF_MULTISEL(msel,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,style,ptbl,cur,cnt)   \
MULTISEL msel = \
{   \
    {OT_MULTISEL, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,style,(void*)ptbl,cnt,cur \
};

#define DEF_EDITFIELD(edit,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,style,pat,maxlen,cursormode,pre,sub,str)   \
EDIT_FIELD edit = \
{   \
    {OT_EDITFIELD, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,maxlen,pat,style,cursormode,0,str,pre,sub, 0\
};

#define DEF_PROGRESSBAR(bar,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,bX,bY,bg,fg,rcl,rct,rcw,rch,min,max,block,pos)	\
PROGRESS_BAR  bar =\
{	\
	{OT_PROGRESSBAR, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },	\
    style,bX,bY,bg,fg,	\
    {rcl,rct,rcw,rch},	\
    min,max,block,pos \
};
 
#define DEF_SCROLLBAR(bar,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,page,thumb,bg,rcl,rct,rcw,rch,max,pos)	\
SCROLL_BAR bar =\
{	\
	{OT_SCROLLBAR, attr, font,ID,lID,rID,tID,dID,   \
	        {l,t,w,h},      \
	        {sh,hl,sel,gry},    \
	        kmap,cb,    \
	        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },	\
    style,page,thumb,bg,	\
    {rcl,rct,rcw,rch},	\
    max,pos \
};

#define DEF_LIST(ls,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,field,numfield,page,bar,intervaly,msel,cnt,ntop,nsel,npos)   \
LIST ls = \
{   \
    {OT_LIST, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    style,(lpLISTFIELD)field,numfield,page,bar,intervaly,(UINT32 *)msel,cnt,ntop,nsel,npos\
};

#define DEF_MATRIXBOX(mb,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,ctype,cnt,ctable,row,col,assigntype,ix,iy,itt,itl,iit,iil,pos)   \
MATRIX_BOX mb = \
{   \
    {OT_MATRIXBOX, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    style,ctype,cnt,(void*)ctable,row,col,assigntype,ix,iy,itt,itl,iit,iil,pos\
};
#endif//_OSDOBJS_DEF_H_


/****************************************************
 *
 * FILE:	zygote_osk_ui.c
 * AUTHOR:	Jason "Zygote" Brownlee
 * DATE:	28/01/2001 - Q3A:1.27g
 * MAIL:	hop_cha@hotmail.com
 * WEB:		http://www.planetquake.com/humandebris
 *
 * This file holds all necessary functions for the 
 * "One Shot Kills" Quake3 Modification.
 * - This is the UI for OSK!
 ****************************************************
 */

#include "ui_local.h"


#define ART_FRAME		"menu/art/addbotframe"
#define ART_BACK0		"menu/art/back_0"
#define ART_BACK1		"menu/art/back_1"	

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menubitmap_s	frame;

	menulist_s		list;

	menubitmap_s	back;

} g_osk_t;

static g_osk_t	g_osk;

#define OSK_TYPE	10
static const char *osktype[] = {
	"Classic Instagib"    ,
	"Rifle Instagib"      ,
	"Bouncing Instagib"   ,
	"Telefrag Instagib"   ,
	"Gauntlet Instagib"   ,
	"Delayed(Classic)"    ,
	"Delayed(Jump 'n Gib)",
	"Delayed(Invisible)"  ,
	"Delayed(Haste)"      ,
	"Delayed(Blast)"      ,
	NULL
};
static const char *oskmode[] = {
	"g_oskmode 1",
	"g_oskmode 2",
	"g_oskmode 3",
	"g_oskmode 4",
	"g_oskmode 5",
	"g_oskmode 6",
	"g_oskmode 7",
	"g_oskmode 8",
	"g_oskmode 9",
	"g_oskmode 10",
	NULL
};


/*
===============
UI_OSK_BackEvent
===============
*/
static void UI_OSK_BackEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}
	UI_PopMenu();
}


/*
=================
UI_OSKMenu_Key
=================
*/
sfxHandle_t UI_OSKMenu_Key( int key ) {
	menulist_s	*l;
	int	x;
	int	y;
	int	index;

	l = (menulist_s	*)Menu_ItemAtCursor( &g_osk.menu );
	if( l != &g_osk.list ) {
		return Menu_DefaultKey( &g_osk.menu, key );
	}

	switch( key ) {
		case K_MOUSE1:
			x = l->generic.left;
			y = l->generic.top;
			if( UI_CursorInRect( x, y, l->generic.right - x, l->generic.bottom - y ) ) {
				index = (uis.cursory - y) / PROP_HEIGHT;
				l->oldvalue = l->curvalue;
				l->curvalue = index;

				if( l->generic.callback ) {
					l->generic.callback( l, QM_ACTIVATED );
					return menu_move_sound;
				}
			}
			return menu_null_sound;

		case K_KP_UPARROW:
		case K_UPARROW:
			l->oldvalue = l->curvalue;

			if( l->curvalue == 0 ) {
				l->curvalue = l->numitems - 1;
			}
			else {
				l->curvalue--;
			}
			return menu_move_sound;

		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			l->oldvalue = l->curvalue;

			if( l->curvalue == l->numitems - 1 ) {
				l->curvalue = 0;;
			}
			else {
				l->curvalue++;
			}
			return menu_move_sound;
	}

	return Menu_DefaultKey( &g_osk.menu, key );
}


/*
=================
UI_OSK_ListDraw
=================
*/
static void UI_OSK_ListDraw( void *self ) {
	menulist_s	*l;
	int			x;
	int			y;
	int			i;
	float		*color;
	qboolean	hasfocus;
	int			style;

	l = (menulist_s *)self;

	hasfocus = (l->generic.parent->cursor == l->generic.menuPosition);

	x =	320;//l->generic.x;
	y =	l->generic.y;
	for( i = 0; i < l->numitems; i++ ) {
		style = UI_LEFT|UI_SMALLFONT|UI_CENTER;
		if( i == l->curvalue ) {
			color = color_yellow;
			if( hasfocus ) {
				style |= UI_PULSE;
			}
		}
		else {
			color = color_orange;
		}

		UI_DrawProportionalString( x, y, l->itemnames[i], style, color );
		y += PROP_HEIGHT;
	}
}


/*
===============
UI_OSK_ListEvent
===============
*/
static void UI_OSK_ListEvent( void *ptr, int event ) {
//	int		id;
	int		selection;
	char	message[256];

	if (event != QM_ACTIVATED)
		return;

//	id = ((menulist_s *)ptr)->generic.id;
	selection = ((menulist_s *)ptr)->curvalue;


	Com_sprintf( message, sizeof(message), oskmode[selection]);


	trap_Cmd_ExecuteText( EXEC_APPEND, va( message ) );
	// UI_PopMenu();
}


/*
===============
UI_OSKMenuInt
===============
*/
static void UI_OSKMenuInt( void ) {
	UI_OSKMenu_Cache();

	memset( &g_osk, 0, sizeof(g_osk) );
	g_osk.menu.fullscreen = qtrue;
	g_osk.menu.wrapAround = qtrue;
	g_osk.menu.showlogo = qtrue;
	g_osk.menu.key = UI_OSKMenu_Key;

//	UI_TeamOrdersMenu_BuildBotList();

	g_osk.banner.generic.type  = MTYPE_BTEXT;
	g_osk.banner.generic.flags = QMF_CENTER_JUSTIFY;
	g_osk.banner.generic.x	    = 320;
	g_osk.banner.generic.y	    = 16;
	g_osk.banner.string  		= "OSK ARENA";
	g_osk.banner.style  	    = UI_CENTER;
	g_osk.banner.color  	    = color_white;

	g_osk.frame.generic.type		= MTYPE_BITMAP;
	g_osk.frame.generic.flags		= QMF_INACTIVE;
	g_osk.frame.generic.name		= ART_FRAME;
	g_osk.frame.generic.x			= 320-233;
	g_osk.frame.generic.y			= 240-166;
	g_osk.frame.width				= 466;
	g_osk.frame.height				= 332;

	g_osk.list.generic.type			= MTYPE_SCROLLLIST;
	g_osk.list.generic.flags		= QMF_HIGHLIGHT_IF_FOCUS;
	g_osk.list.generic.ownerdraw	= UI_OSK_ListDraw;
	g_osk.list.generic.callback		= UI_OSK_ListEvent;
	g_osk.list.generic.x			= 320-64;
	g_osk.list.generic.y			= 120;
//	g_osk.list.generic.id = id;
	g_osk.list.numitems				= OSK_TYPE;
	g_osk.list.itemnames			= osktype;


	g_osk.back.generic.type		= MTYPE_BITMAP;
	g_osk.back.generic.name		= ART_BACK0;
	g_osk.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	g_osk.back.generic.callback	= UI_OSK_BackEvent;
	g_osk.back.generic.x			= 0;
	g_osk.back.generic.y			= 480-64;
	g_osk.back.width				= 128;
	g_osk.back.height				= 64;
	g_osk.back.focuspic			= ART_BACK1;

	Menu_AddItem( &g_osk.menu, &g_osk.banner );
	Menu_AddItem( &g_osk.menu, &g_osk.frame );
	Menu_AddItem( &g_osk.menu, &g_osk.list );
	Menu_AddItem( &g_osk.menu, &g_osk.back );

	g_osk.list.generic.left = 220;
	g_osk.list.generic.top = g_osk.list.generic.y;
	g_osk.list.generic.right = 420;
	g_osk.list.generic.bottom = g_osk.list.generic.top + g_osk.list.numitems * PROP_HEIGHT;
//	UI_TeamOrdersMenu_SetList( ID_LIST_BOTS );
	g_osk.list.curvalue = (trap_Cvar_VariableValue("g_oskmode") -1);
}


/*
=================
UI_OSKMenu_Cache
=================
*/
void UI_OSKMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_FRAME );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
}




/*
=================
UI_OSKMenu
=================
*/
void UI_OSKMenu( void ) {
	UI_OSKMenuInt();
	UI_PushMenu( &g_osk.menu );
}						  

#ifndef DRAW_H
#define DRAW_H

#include "extc.h"

#ifdef _WIN32

typedef GdiFont PlatformFont;
#define set_font nk_gdi_set_font
#elif defined(__linux__)

typedef struct nk_user_font PlatformFont;
#define set_font nk_xlib_set_font
#else
#error Unsupported platform
#endif

#include <stddef.h>
#include <stdbool.h>

// fixed ratio layout 
float ratio[] = { 0.3, 0.40, 0.3 };
float ratioSingle[] = { 0.3, 0.7 };
float ratioEqual[] = { 0.5, 0.5 };
float ratioFull[] = { 1.0 };

#ifdef _WIN32
void data_debug_style(struct nk_context* ctx, GdiFont* fontSmall, int w, int h, struct appC app, struct userC user, struct dataEntryC* entries, size_t count)
#elif __linux__
void data_debug_style(struct nk_context* ctx, struct nk_font* fontSmall, int w, int h, struct appC app, struct userC user, struct dataEntryC* entries, size_t count)
#endif
{
	nk_style_default(ctx);
	ext_BG(ctx, 0, 0, 0);
	#ifdef _WIN32
	nk_gdi_set_font(fontSmall);
	#elif __linux__
	nk_style_set_font(ctx, &fontSmall->handle);
	#endif

	// Semi-transparent black background
	ctx->style.window.background = nk_rgba(0, 0, 0, 0);
	ctx->style.text.color = nk_rgb(255, 255, 255);
	ctx->style.window.padding.x = 4;
	ctx->style.window.padding.y = 3;
	ctx->style.window.spacing.x = 0;  // horizontal spacing between widgets
	ctx->style.window.spacing.y = 3;  // vertical spacing between rows


	if (nk_begin(ctx, "[color=#FF0000]DEBUG[/color]", nk_rect(0, 0, w, h), NK_WINDOW_TITLE | NK_WINDOW_BACKGROUND))
	{
		#ifdef _WIN32
			nk_gdi_set_font(fontSmall);
		#elif __linux__
			nk_style_set_font(ctx, &fontSmall->handle);
		#endif
		// ui
				// Print app struct
		nk_layout_row(ctx, NK_DYNAMIC, 12, 1, ratioFull);
		nk_label(ctx, custom_strcat_static(1, "[color=#FFFF00]struct appC[/color]"), NK_TEXT_ALIGN_LEFT);

		nk_label(ctx, custom_strcat_static(3,"[color=#FFFF00]osuId[/color]:[color=#FF0000]",intToConst(app.osuId),"[/color]"), NK_TEXT_ALIGN_LEFT);

		nk_label(ctx, custom_strcat_static(3,
			"[color=#FFFF00]gameMode[/color]: [color=#FF0000]",
			intToConst((int)app.gameMode),
			"[/color]"), NK_TEXT_ALIGN_LEFT);

		nk_label(ctx, custom_strcat_static(3,
			"[color=#FFFF00]server[/color]: [color=#FF0000]",
			intToConst((int)app.server),
			"[/color]"), NK_TEXT_ALIGN_LEFT);

		nk_label(ctx, custom_strcat_static(1, "[color=#FFFFFF]------------------------------[/color]"), NK_TEXT_ALIGN_LEFT);

		// Print user struct
		nk_layout_row(ctx, NK_DYNAMIC, 12, 1, ratioFull);
		nk_label(ctx, custom_strcat_static(1, "[color=#FFFF00]struct userC[/color]"), NK_TEXT_ALIGN_LEFT);

		nk_label(ctx, custom_strcat_static(3,
			"[color=#FFFF00]username[/color]: [color=#FF0000]",
			user.username ? user.username : "",
			"[/color]"), NK_TEXT_ALIGN_LEFT);

		nk_label(ctx, custom_strcat_static(3,
			"[color=#FFFF00]avatar[/color]: [color=#FF0000]",
			user.avatar ? user.avatar : "",
			"[/color]"), NK_TEXT_ALIGN_LEFT);

		nk_label(ctx, custom_strcat_static(1, "[color=#FFFFFF]------------------------------[/color]"), NK_TEXT_ALIGN_LEFT);

		nk_layout_row(ctx, NK_DYNAMIC, 12, 1, ratioFull);
		nk_label(ctx, custom_strcat_static(1, "[color=#FFFF00]struct dataEntryC* entries[/color]"), NK_TEXT_ALIGN_LEFT);
		for (size_t i = 0; i < count; i++) {
			nk_layout_row(ctx, NK_DYNAMIC, 12, 1, ratioFull);
			nk_label(ctx, custom_strcat_static(1,"[color=#FFFFFF]------------------------------[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3,"[color=#FFFF00]\"[/color][color=#0000FF]key[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", entries[i].key, "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3,"[color=#FFFF00]\"[/color][color=#0000FF]init[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", entries[i].init, "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3,"[color=#FFFF00]\"[/color][color=#0000FF]current[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", entries[i].current, "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3,"[color=#FFFF00]\"[/color][color=#0000FF]change[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", entries[i].change, "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3,"[color=#FFFF00]\"[/color][color=#0000FF]sort[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", intToConst(entries[i].sort), "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3, "[color=#FFFF00]\"[/color][color=#0000FF]positive[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", bool2str(entries[i].positive), "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3, "[color=#FFFF00]\"[/color][color=#0000FF]display[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", bool2str(entries[i].display), "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3, "[color=#FFFF00]\"[/color][color=#0000FF]single[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", bool2str(entries[i].single), "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3, "[color=#FFFF00]\"[/color][color=#0000FF]banchoSupport[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", bool2str(entries[i].banchoSupport), "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
			nk_label(ctx, custom_strcat_static(3, "[color=#FFFF00]\"[/color][color=#0000FF]titanicSupport[/color][color=#FFFF00]\"[/color][color=#FFFFFF]:([/color][color=#FF0000]", bool2str(entries[i].titanicSupport), "[/color][color=#FFFFFF])[/color]"), NK_TEXT_ALIGN_LEFT);
		}
	}
}

#ifdef _WIN32
void drawContent(struct nk_context* ctx, PlatformFont* font, PlatformFont* fontSmall, PlatformFont* fontHeader, int w, int h, struct appC app, struct userC user, struct dataEntryC* entries, size_t count, bool debug, bool data_debug) {
#elif __linux__
void drawContent(struct nk_context* ctx, struct nk_font* font, struct nk_font* fontSmall, struct nk_font* fontHeader, int w, int h, struct appC app, struct userC user, struct dataEntryC* entries, size_t count, bool debug, bool data_debug) {
#endif
	/* GUI */
	#ifdef _WIN32
	nk_gdi_set_font(fontHeader);
	#elif __linux__
	nk_style_set_font(ctx, &fontHeader->handle);
	#endif
	if (!data_debug) {
		nk_draw_set_color_inline(ctx, NK_COLOR_INLINE_TAG);

		ctx->style.window.header.active = nk_style_item_color(nk_rgb(21, 133, 181));

		ext_TextColor(ctx, 255, 255, 255);
		ext_BG(ctx, 35, 35, 35);

		// scrollbar bg
		ctx->style.scrollv.hover.data.color = nk_rgb(0, 0, 0);
		ctx->style.scrollv.normal.data.color = nk_rgb(0, 0, 0);
		ctx->style.scrollv.active.data.color = nk_rgb(0, 0, 0);

		// scrollbar fg
		ctx->style.scrollv.cursor_hover.data.color = nk_rgb(255, 255, 255);
		ctx->style.scrollv.cursor_active.data.color = nk_rgb(255, 255, 255);

		//border
		ctx->style.edit.border_color = nk_rgb(200, 200, 200);

		// text box
		ctx->style.edit.text_normal = nk_rgb(255, 255, 255);
		ctx->style.edit.text_hover = nk_rgb(0, 0, 0);
		ctx->style.edit.text_active = nk_rgb(0, 0, 0);

		// body
		ctx->style.edit.normal.data.color = nk_rgb(13, 13, 13);
		ctx->style.edit.hover.data.color = nk_rgb(255, 255, 255);
		ctx->style.edit.active.data.color = nk_rgb(255, 255, 255);

		// tab
		ctx->style.tab.background.data.color = nk_rgb(0, 0, 0);
		ctx->style.tab.text = nk_rgb(255, 255, 255);

		ctx->style.tab.tab_minimize_button.active.data.color = nk_rgb(0, 0, 0);
		ctx->style.tab.tab_minimize_button.hover.data.color = nk_rgb(0, 0, 0);
		ctx->style.tab.tab_minimize_button.normal.data.color = nk_rgb(0, 0, 0);
		
		ctx->style.tab.tab_minimize_button.text_normal = nk_rgb(255, 255, 255);
		ctx->style.tab.tab_minimize_button.text_normal = nk_rgb(255, 255, 255);
		ctx->style.tab.tab_minimize_button.text_normal = nk_rgb(255, 255, 255);

		ctx->style.tab.tab_maximize_button.active.data.color = nk_rgb(0, 0, 0);
		ctx->style.tab.tab_maximize_button.hover.data.color = nk_rgb(0, 0, 0);
		ctx->style.tab.tab_maximize_button.normal.data.color = nk_rgb(0, 0, 0);
		
		ctx->style.tab.tab_maximize_button.text_normal = nk_rgb(255, 255, 255);
		ctx->style.tab.tab_maximize_button.text_normal = nk_rgb(255, 255, 255);
		ctx->style.tab.tab_maximize_button.text_normal = nk_rgb(255, 255, 255);

		ctx->style.tab.indent = 0;
		ctx->style.tab.border = 0;

	}
	else {
		data_debug_style(ctx, fontSmall, w,h, app, user, entries, count);
		return;
	}

	// scrollbar width
	ctx->style.window.scrollbar_size.x = 8.0f;
	ctx->style.window.padding.x = 10.0f;

	if (nk_begin(ctx, custom_strcat_static(3, "[color=#FFFFFF]", user.username ? user.username : "", "[/color]"), nk_rect(0, 0, w, h),NK_WINDOW_TITLE| NK_WINDOW_BACKGROUND))
	{	
		int nkEditHeight = 0;
		#ifdef _WIN32
			nk_gdi_set_font(font);
			nkEditHeight = 20;
		#elif __linux__
			nk_style_set_font(ctx, &font->handle);
			nkEditHeight = 25;
		#endif
			if (debug) {
				//border
				ctx->style.edit.border_color = nk_rgb(0, 0, 0);

				// text box
				ctx->style.edit.text_normal = nk_rgb(255, 255, 255);
				ctx->style.edit.text_hover = nk_rgb(0, 0, 0);
				ctx->style.edit.text_active = nk_rgb(0, 0, 0);

				if (nk_tree_push(ctx, NK_TREE_TAB, "Info Debug", NK_MINIMIZED)) {
					nk_layout_row(ctx, NK_DYNAMIC, nkEditHeight, 2, ratioEqual);
					nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, "WIDTH", 30, nk_filter_default);
					nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, intToConst(width), 30, nk_filter_default);
					nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, "HEIGHT", 30, nk_filter_default);
					nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, intToConst(height), 30, nk_filter_default);
					nk_tree_pop(ctx);
				}
				//border
				ctx->style.edit.border_color = nk_rgb(200, 200, 200);

				// text box
				ctx->style.edit.text_normal = nk_rgb(255, 255, 255);
				ctx->style.edit.text_hover = nk_rgb(0, 0, 0);
				ctx->style.edit.text_active = nk_rgb(0, 0, 0);
			}

		for (size_t i = 0; i < count; i++) {

			if (!entries[i].display)
				continue;

			switch (app.server) {
				case bancho:
					if (!entries[i].banchoSupport)
						continue;
					break;
				case titanic:
					if (!entries[i].titanicSupport)
						continue;
					break;
			}

			if (entries[i].single) {
				nk_layout_row(ctx, NK_DYNAMIC, nkEditHeight, 2, ratioSingle);
			}
			else {
				nk_layout_row(ctx, NK_DYNAMIC, nkEditHeight, 3, ratio);
			}

			nk_label(ctx, entries[i].name, NK_TEXT_ALIGN_LEFT);

			nk_edit_string_zero_terminated(
				ctx,
				NK_EDIT_SELECTABLE | NK_EDIT_CLIPBOARD,
				entries[i].current,
				30,
				nk_filter_default
			);
			if (!entries[i].single) {
				if (entries[i].positive) {
					ctx->style.edit.text_normal = nk_rgb(0, 255, 0);
				}
				else {
					ctx->style.edit.text_normal = nk_rgb(255, 0, 0);
				}
				nk_edit_string_zero_terminated(
					ctx,
					NK_EDIT_SELECTABLE | NK_EDIT_CLIPBOARD,
					entries[i].change,
					30,
					nk_filter_default
				);
				ctx->style.edit.text_normal = nk_rgb(255, 255, 255);
			}
		}
	}
};

#endif // DRAW_H

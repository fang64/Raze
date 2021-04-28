//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT
Copyright (C) 2019-2021 Christoph Oelckers

This file is part of Raze.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------


class BloodIntroImage : ImageScreen
{
	bool mus;

	ScreenJob Init(String texture, int flags = 0, bool withmusic = false)
	{
		Super.InitNamed(texture, flags);
		mus = withmusic;
		return self;
	}

	override void Start()
	{
		Blood.sndStartSampleNamed("THUNDER2", 128, -1);
		if (mus) Blood.PlayIntroMusic(); // this is script defined.
	}
}

struct BloodScreen
{
	enum EConstants
	{
		kLoadScreenWideBackWidth = 256,
		kLoadScreenWideSideWidth = 128,
	}

	//---------------------------------------------------------------------------
	//
	//
	//
	//---------------------------------------------------------------------------

	static void DrawBackground()
	{
		if (Blood.OriginalLoadScreen() && Screen.GetAspectRatio() >= 1.34)
		{
			int width = screen.GetWidth() * 240 / screen.GetHeight();
			int nCount = (width + kLoadScreenWideBackWidth - 1) / kLoadScreenWideBackWidth;
			for (int i = 0; i < nCount; i++)
			{
				Screen.DrawTexture(TexMan.CheckForTexture("LoadScreenWideBack"), false, (i * kLoadScreenWideBackWidth), 0, DTA_VirtualWidth, width, DTA_VirtualHeight, 200, DTA_KeepRatio, true);
			}
			Screen.DrawTexture(TexMan.CheckForTexture("LoadScreenWideLeft"), false, 0, 0, DTA_VirtualWidth, width, DTA_VirtualHeight, 200, DTA_KeepRatio, true, DTA_TopLeft, true);
			let texid = TexMan.CheckForTexture("LoadScreenWideRight");
			let size = TexMan.GetScaledSize(texid);
			Screen.DrawTexture(texid, false, width - size.x, 0, DTA_TopLeft, true,	DTA_VirtualWidth, width, DTA_VirtualHeight, 200, DTA_KeepRatio, true);
			texid = TexMan.CheckForTexture("LoadScreenWideMiddle");
			size = TexMan.GetScaledSize(texid);
			Screen.DrawTexture(texid, false, (width - size.x) / 2, 0, DTA_TopLeft, true, DTA_VirtualWidth, width, DTA_VirtualHeight, 200, DTA_KeepRatio, true);
		}
		else
		{
			Screen.DrawTexture(TexMan.CheckForTexture("LoadScreen"), false, 0, 0, DTA_FullscreenEx, FSMode_ScaleToFit43);
		}
	}

	//---------------------------------------------------------------------------
	//
	// 
	//
	//---------------------------------------------------------------------------

	static int DrawCaption(String title, int y, bool drawit)
	{
		let font = generic_ui? NewConsoleFont : BigFont;
		let texid = TexMan.CheckForTexture("MENUBAR");
		let texsize = TexMan.GetScaledSize(texid);
		let fonth = font.GetGlyphHeight("A");
		if (drawit)
		{
			int width = font.StringWidth(title);
			if (texid.isValid())
			{
				double scalex = 1.; // Expand the box if the text is longer
				if (texsize.X - 10 < width) scalex = width / (texsize.X - 10);
				screen.DrawTexture(texid, false, 160, 20, DTA_FullscreenScale, FSMode_Fit320x200Top, DTA_CenterOffsetRel, true, DTA_ScaleX, scalex);
			}
			screen.DrawText(font, Font.CR_UNDEFINED, 160 - width / 2, 20 - fonth / 2, title, DTA_FullscreenScale, FSMode_Fit320x200Top);
		}
		double fx, fy, fw, fh;
		[fx, fy, fw, fh] = Screen.GetFullscreenRect(320, 200, FSMode_ScaleToFit43Top);
		int h = texid.isValid() && texsize.Y < 40? texsize.Y : fonth;
		return int((y+h) * fh / 200); // This must be the covered height of the header in true pixels.
	}

	//---------------------------------------------------------------------------
	//
	// 
	//
	//---------------------------------------------------------------------------

	static void DrawText(Font pFont, String pString, int x, int y, int position = 0, int nShade = 0, int nPalette = 0, bool shadow = true, float alpha = 1.)
	{
		if (position > 0) x -= pFont.StringWidth(pString) * position / 2;
		if (shadow) Screen.DrawText(pFont, Font.CR_UNDEFINED, x+1, y+1, pString, DTA_FullscreenScale, FSMode_Fit320x200, DTA_Color, 0xff000000, DTA_Alpha, 0.5);
		Screen.DrawText(pFont, Font.CR_UNDEFINED, x, y, pString, DTA_FullscreenScale, FSMode_Fit320x200, DTA_TranslationIndex, Translation.MakeID(Translation_Remap, nPalette),
				DTA_Color, Raze.shadeToLight(nShade), DTA_Alpha, alpha);

	}

}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

class BloodSummaryScreen : SummaryScreenBase
{
	ScreenJob Init(MapRecord map, SummaryInfo info)
	{
		Super.Init(fadein|fadeout);
		SetParameters(map, info);
		return self;
	}

	override void Start()
	{
		Blood.sndStartSample(268, 128, -1, false, CHANF_UI);
	}

	override bool OnEvent(InputEvent ev)
	{
		if (ev.type == InputEvent.Type_KeyDown && !Raze.specialKeyEvent(ev))
		{
			jobstate = skipped;
			return true;
		}
		return false;
	}

	void DrawKills()
	{
		String pBuffer;
		BloodScreen.DrawText(BigFont, Stringtable.Localize("$KILLS") .. ":", 75, 50);
		pBuffer = String.Format("%2d", stats.Kills);
		BloodScreen.DrawText(BigFont, pBuffer, 160, 50);
		BloodScreen.DrawText(BigFont, "$OF", 190, 50);
		pBuffer = String.Format( "%2d", stats.MaxKills);
		BloodScreen.DrawText(BigFont, pBuffer, 220, 50);
	}

	void DrawSecrets()
	{
		String pBuffer;
		BloodScreen.DrawText(BigFont, StringTable.Localize("$TXT_SECRETS") .. ":", 75, 70);
		pBuffer = String.Format( "%2d", stats.secrets);
		BloodScreen.DrawText(BigFont, pBuffer, 160, 70);
		BloodScreen.DrawText(BigFont, "$OF", 190, 70);
		pBuffer = String.Format( "%2d", stats.maxsecrets);
		BloodScreen.DrawText(BigFont, pBuffer, 220, 70);
		if (stats.SuperSecrets > 0) BloodScreen.DrawText(BigFont, "$TXT_SUPERSECRET", 160, 100, 1, 0, 2);
	}

	override void Draw(double sm)
	{
		BloodScreen.DrawBackground();
		BloodScreen.DrawCaption("$TXTB_LEVELSTATS", 0, true);
		if (stats.cheated)
		{
			let text = StringTable.Localize("$TXTB_CHEATED");
			let font = SmallFont2.CanPrint(text)? SmallFont2 : SmallFont;
			BloodScreen.DrawText(font, text, 160, 32, 1, shadow:font == SmallFont2);
		}
		DrawKills();
		DrawSecrets();

		int myclock = ticks * 120 / GameTicRate;
		if ((myclock & 32))
		{
			let text = StringTable.Localize("$PRESSKEY");
			let font = SmallFont2.CanPrint(text)? SmallFont2 : SmallFont;
			BloodScreen.DrawText(font, text, 160, 134, 1, shadow:font == SmallFont2);
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

class BloodMPSummaryScreen : SkippableScreenJob
{
	int numplayers;
	ScreenJob Init(int numplayer)
	{
		Super.Init(fadein|fadeout);
		numplayers = numplayer;
		return self;
	}

	override void Start()
	{
		Blood.sndStartSample(268, 128, -1, false, CHANF_UI);
	}

	void DrawKills()
	{
		String pBuffer;
		BloodScreen.DrawText(SmallFont2, "#", 85, 35);
		BloodScreen.DrawText(SmallFont2, "$NAME", 100, 35);
		BloodScreen.DrawText(SmallFont2, "$FRAGS", 210, 35);

		for (int i = 0; i < numplayers; i++)
		{
			pBuffer = String.Format( "%-2d", i);
			BloodScreen.DrawText(SmallFont2,  pBuffer, 85, 50 + 8 * i);
			pBuffer = String.Format( "%s", Raze.PlayerName(i));
			BloodScreen.DrawText(SmallFont2,  pBuffer, 100, 50 + 8 * i);
			pBuffer = String.Format( "%d", Raze.playerFrags(i, -1));
			BloodScreen.DrawText(SmallFont2,  pBuffer, 210, 50 + 8 * i);
		}
	}

	override void Draw(double sr) 
	{
		BloodScreen.DrawBackground();
		BloodScreen.DrawCaption("$TXTB_FRAGSTATS", 0, true);
		DrawKills();

		int myclock = ticks * 120 / GameTicRate;
		if ((myclock & 32))
		{
			let text = StringTable.Localize("$PRESSKEY");
			let font = SmallFont2.CanPrint(text)? SmallFont2 : SmallFont;
			BloodScreen.DrawText(font, text, 160, 134, 1, shadow:font == SmallFont2);
		}
	}
}

//---------------------------------------------------------------------------
//
// 
//
//---------------------------------------------------------------------------

class BloodLoadScreen : ScreenJob
{ 
	String loadtext;
	MapRecord rec;

	ScreenJob Init(MapRecord maprec)
	{
		Super.Init(fadein);

		/*if (gGameOptions.nGameType == 0)*/ loadtext = "$TXTB_LLEVEL";
		//else loadText = String.Format("$TXTB_NETGT%d", gGameOptions.nGameType));

		rec = maprec;
		return self;
	}

	override void OnTick()
	{
		if (fadestate == visible) jobstate = finished;
	}
	
	override void Draw(double sr) 
	{
		BloodScreen.DrawBackground();
		BloodScreen.DrawCaption(loadtext, 0, true);
		BloodScreen.DrawText(BigFont, rec.DisplayName(), 160, 50, 1);

		let text = StringTable.Localize("$TXTB_PLSWAIT");
		let font = SmallFont2.CanPrint(text)? SmallFont2 : SmallFont;
		BloodScreen.DrawText(font, text, 160, 134, 1, shadow:font == SmallFont2);
	}
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

class BloodCutscenes
{
	static void BuildIntro(ScreenJobRunner runner)
	{
		if (!userConfig.nologo)
		{
			Array<int> soundinfo;
			if (Wads.CheckNumForFullName("logo.smk") != -1)
			{
				String s = "logo.wav"; // sound name must be converted at run-time, not compile time!
				runner.Append(MoviePlayerJob.CreateWithSound("logo.smk", s, MoviePlayer.FIXEDVIEWPORT));
			}
			else
			{
				runner.Append(new("BloodIntroImage").Init("MonolithScreen", ScreenJob.fadein|ScreenJob.fadeout));
 			}
			if (Wads.CheckNumForFullName("gti.smk") != -1)
			{
				String s = "gt.wav";
				runner.Append(MoviePlayerJob.CreateWithSound("gti.smk", s, MoviePlayer.FIXEDVIEWPORT));
			}
			else
			{
				runner.Append(new("BloodIntroImage").Init("GTIScreen", ScreenJob.fadein|ScreenJob.fadeout));
			}
		}
		runner.Append(new("BloodIntroImage").Init("Titlescreen", ScreenJob.fadein, true));
	}

	//---------------------------------------------------------------------------
	//
	// 
	//
	//---------------------------------------------------------------------------

	static void BuildMPSummary(ScreenJobRunner runner, MapRecord map, SummaryInfo stats)
	{
		runner.Append(new("BloodMPSummaryScreen").Init(stats.playercount));
	}

	//---------------------------------------------------------------------------
	//
	// 
	//
	//---------------------------------------------------------------------------

	static void BuildSPSummary(ScreenJobRunner runner, MapRecord map, SummaryInfo stats)
	{
		runner.Append(new("BloodSummaryScreen").Init(map, stats));
	}

	//---------------------------------------------------------------------------
	//
	// 
	//
	//---------------------------------------------------------------------------

	static void BuildLoading(ScreenJobRunner runner, MapRecord map)
	{
		runner.Append(new("BloodLoadScreen").Init(map));
	}
}

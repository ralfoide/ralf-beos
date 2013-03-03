/*****************************************************************************

	Projet	: Pulsar

	Fichier:	CPulsarWin.h
	Partie	: Loader

	Auteur	: RM
	Format	: tabs==2

*****************************************************************************/

#ifndef _H_CPULSAR_MSG_
#define _H_CPULSAR_MSG_

//---------------------------------------------------------------------------
// some messages

#define K_MSG_STATE_STREAM	'stST'
#define K_MSG_START_STREAM	'saST'
#define K_MSG_STOP_STREAM		'soST'

#define K_MSG_FILTER_INFO		'fInf'
#define K_NAME_FILTER_INFO	"filter"		// drag filter from the addon list
#define K_NAME_FILTER_LINE	"line"			// drag filter/line from the track view
#define K_NAME_FILTER_OVER	"over"			// drag filter/line from the track view

#define K_MSG_FFT_WINDOW						'fftW'
#define K_NAME_FFT_WINDOW						"index"

#define K_MSG_QUIT									'btQt'
#define K_MSG_START_BTN							'staB'
#define K_MSG_STOP_BTN							'stpB'
#define K_MSG_ADC_BTN								'adcB'
#define K_MSG_DAC_BTN								'dacB'
#define K_MSG_START_STREAM_BTN			'staS'
#define K_MSG_END_STREAM_BTN				'endS'
#define K_MSG_FFT_CHECK							'fftC'
#define K_MSG_FILL_CHECK						'filC'
#define K_MSG_PREF									'pReF'
#define K_MSG_ABOUT									'abot'
#define K_MSG_ABOUT2								'abo2'
#define K_MSG_INTEFACE_BTN					'intr'
#define K_MSG_FULLSCREEN_BTN				'gKit'
#define K_MSG_PREVIEW_BTN						'preW'
#define K_MSG_IDEAL_RATE						'ilFr'
#define K_MSG_8_BPP									'bp08'
#define K_MSG_16_BPP								'bp16'
#define K_MSG_32_BPP								'bp32'
#define K_MSG_128_BPP								'bp12'
#define K_MSG_CD_PLAY								'cdPL'
#define K_MSG_CD_STOP								'cdST'
#define K_MSG_CD_EJECT							'cdEJ'
#define K_MSG_CD_PAUSE							'cdPA'
#define K_MSG_CD_PREV								'cdPR'
#define K_MSG_CD_NEXT								'cdNE'
#define K_MSG_CD_DEVICE							'cdDV'
#define K_MSG_CD_TRACK							'cdTK'
#define K_MSG_SOUND_PANEL						'BeSn'
#define K_MSG_RELOAD_ADDON					'reAD'
#define K_MSG_PREPARE_TRACK_LINE		'prTl'
#define K_MSG_TERMINATE_TRACK_LINE	'teTl'

#define K_MSG_NEW										'doNW'
#define K_MSG_LOAD									'doLO'
#define K_MSG_SAVE									'doSA'
#define K_MSG_SAVE_AS								'doAS'
#define K_LOAD_REQUESTED						'ldRQ'

#define K_MSG_CDINPUT_BTN						'cdIN'
#define K_MSG_LINEIN_BTN						'LiIN'
#define K_MSG_AIFFINPUT_BTN					'aiIN'
#define K_MSG_MP3INPUT_BTN					'm3IN'
#define K_MSG_MODINPUT_BTN					'moIN'
#define K_MSG_MICINPUT_BTN					'miIN'

#define K_MSG_ADDON_ITEM_INVOKED		'adIV'
#define K_MSG_ADDON_ITEM_SELECTED		'adSL'

#define K_MSG_CUT										B_CUT
#define K_MSG_COPY									B_COPY
#define K_MSG_PASTE									B_PASTE
#define K_MSG_SELECT_ALL						B_SELECT_ALL
#define K_MSG_DESELECT							'kdSe'
#define K_MSG_DELETE								'kDel'
#define K_MSG_UNDO									'kUnd'
#define K_MSG_REDO									'kRed'


#define K_MSG_LOAD_CONFIG						'lCfg'
#define K_NAME_CONFIG_INDEX					"index"

//---------------------------------------------------------------------------

#endif // of _H_CPULSAR_MSG_

// eoh

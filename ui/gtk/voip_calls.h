/* voip_calls.h
 * VoIP calls summary addition for Wireshark
 *
 * Copyright 2004, Ericsson , Spain
 * By Francisco Alcoba <francisco.alcoba@ericsson.com>
 *
 * based on h323_calls.h
 * Copyright 2004, Iskratel, Ltd, Kranj
 * By Miha Jemec <m.jemec@iskratel.si>
 *
 * H323, RTP and Graph Support
 * By Alejandro Vaquero, alejandro.vaquero@verso.com
 * Copyright 2005, Verso Technologies Inc.
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation,  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __VOIP_CALLS_H__
#define __VOIP_CALLS_H__

#include <glib.h>
#include <stdio.h>
#include <epan/address.h>
#include <epan/guid-utils.h>
#include <epan/tap-voip.h>

/****************************************************************************/
extern const char *voip_call_state_name[8];

typedef enum _voip_protocol {
    VOIP_SIP,
    VOIP_ISUP,
    VOIP_H323,
    VOIP_MGCP,
    VOIP_AC_ISDN,
    VOIP_AC_CAS,
    MEDIA_T38,
    TEL_H248,
    TEL_SCCP,
    TEL_BSSMAP,
    TEL_RANAP,
    VOIP_UNISTIM,
    VOIP_SKINNY,
    VOIP_IAX2,
    VOIP_COMMON
} voip_protocol;

typedef enum _hash_indexes {
    SIP_HASH=0
} hash_indexes;

extern const char *voip_protocol_name[];

typedef enum _flow_show_options
{
    FLOW_ALL,
    FLOW_ONLY_INVITES
} flow_show_options;

flow_show_options VoIPcalls_get_flow_show_option(void);
void VoIPcalls_set_flow_show_option(flow_show_options option);

/** defines specific SIP data */

typedef enum _sip_call_state {
    SIP_INVITE_SENT,
    SIP_200_REC,
    SIP_CANCEL_SENT
} sip_call_state;

typedef struct _sip_calls_info {
    gchar *call_identifier;
    guint32 invite_cseq;
    sip_call_state sip_state;
} sip_calls_info_t;

/** defines specific ISUP data */
typedef struct _isup_calls_info {
    guint16			cic;
    guint32			opc, dpc;
    guint8			ni;
} isup_calls_info_t;

/* defines specific H245 data */
typedef struct _h245_address {
    address h245_address;
    guint16 h245_port;
} h245_address_t;

/** defines specific H323 data */
typedef struct _h323_calls_info {
    e_guid_t *guid;	/* Call ID to identify a H225 */
    GList*  h245_list;				/**< list of H245 Address and ports for tunneling off calls*/
    address h225SetupAddr;			/**< we use the SETUP H225 IP to determine if packets are forward or reverse */
    gboolean is_h245;
    gboolean is_faststart_Setup;	/**< if faststart field is included in Setup*/
    gboolean is_faststart_Proc;		/**< if faststart field is included in Proce, Alerting, Progress or Connect*/
    gboolean is_h245Tunneling;
    gint32 q931_crv;
    gint32 q931_crv2;
    guint requestSeqNum;
} h323_calls_info_t;

/**< defines specific MGCP data */
typedef struct _mgcp_calls_info {
    gchar *endpointId;
    gboolean fromEndpoint; /**< true if the call was originated from the Endpoint, false for calls from MGC */
} mgcp_calls_info_t;

/** defines specific ACTRACE ISDN data */
typedef struct _actrace_isdn_calls_info {
    gint32 crv;
    int trunk;
} actrace_isdn_calls_info_t;

/** defines specific ACTRACE CAS data */
typedef struct _actrace_cas_calls_info {
    gint32 bchannel;
    int trunk;
} actrace_cas_calls_info_t;

/** defines specific SKINNY data */
typedef struct _skinny_calls_info {
    guint32 callId;
} skinny_calls_info_t;

/** defines a voip call */
typedef struct _voip_calls_info {
    voip_call_state call_state;
    voip_call_active_state call_active_state;
    gchar *call_id;
    gchar *from_identity;
    gchar *to_identity;
    gpointer prot_info;
    void(*free_prot_info)(gpointer);
    address initial_speaker;
    guint32 npackets;
    voip_protocol protocol;
    gchar *protocol_name;
    gchar *call_comment;
    guint16 call_num;
    /**> The frame_data struct holds the frame number and timing information needed. */
    frame_data *start_fd;
    nstime_t start_rel_ts;
    frame_data *stop_fd;
    nstime_t stop_rel_ts;
    gboolean selected;

} voip_calls_info_t;

/**
 * structure that holds the information about all detected calls */
/* struct holding all information of the tap */

typedef struct _voip_calls_tapinfo {
    int     ncalls;							/**< number of call */
    GQueue*  callsinfos;					/**< queue with all calls */
    GHashTable* callsinfo_hashtable[1];		/**< array of hashes per voip protocol; currently only the one for SIP is used */
    int     npackets;						/**< total number of packets of all calls */
    voip_calls_info_t* filter_calls_fwd;	/**< used as filter in some tap modes */
    guint32 launch_count;					/**< number of times the tap has been run */
    int start_packets;
    int completed_calls;
    int rejected_calls;
    seq_analysis_info_t* graph_analysis;
    epan_t *session;					/**< epan session */
    gboolean redraw;
} voip_calls_tapinfo_t;



/* structure that holds the information about all RTP streams associated with the calls */
/** struct holding all information of the RTP tap */
typedef struct _voip_rtp_tapinfo {
    int     nstreams;       /**< number of rtp streams */
    GList*  list;			/**< list with the rtp streams */
} voip_rtp_tapinfo_t;

/****************************************************************************/
/* INTERFACE */

/**
 * Registers the voip_calls tap listeners (if not already done).
 * From that point on, the calls list will be updated with every redissection.
 * This function is also the entry point for the initialization routine of the tap system.
 * So whenever voip_calls.c is added to the list of WIRESHARK_TAP_SRCs, the tap will be registered on startup.
 * If not, it will be registered on demand by the voip_calls functions that need it.
 */
void actrace_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void h225_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void h245dg_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void h248_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void iax2_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void isup_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void mgcp_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void mtp3_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void q931_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void rtp_event_init_tap(voip_calls_tapinfo_t *tap_id_base);
void rtp_init_tap(voip_calls_tapinfo_t *tap_id_base);
void sccp_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void sdp_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void sip_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void skinny_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void t38_init_tap(voip_calls_tapinfo_t *tap_id_base);
void unistim_calls_init_tap(voip_calls_tapinfo_t *tap_id_base);
void VoIPcalls_init_tap(voip_calls_tapinfo_t *tap_id_base);

/**
 * Removes the voip_calls tap listener (if not already done)
 * From that point on, the voip calls list won't be updated any more.
 */
void remove_tap_listener_actrace_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_h225_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_h245dg_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_h248_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_iax2_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_isup_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_mgcp_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_mtp3_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_q931_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_rtp(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_rtp_event(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_sccp_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_sdp_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_sip_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_skinny_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_t38(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_unistim_calls(voip_calls_tapinfo_t *tap_id_base);
void remove_tap_listener_voip_calls(voip_calls_tapinfo_t *tap_id_base);

/**
 * Retrieves a constant reference to the unique info structure of the voip_calls tap listener.
 * The user should not modify the data pointed to.
 */
voip_calls_tapinfo_t* voip_calls_get_info(void);

/**
 * Cleans up memory of voip calls tap.
 */
void isup_calls_reset(voip_calls_tapinfo_t *tapinfo);
void mtp3_calls_reset(voip_calls_tapinfo_t *tapinfo);
void q931_calls_reset(voip_calls_tapinfo_t *tapinfo);
void voip_calls_reset(voip_calls_tapinfo_t *tapinfo);

void graph_analysis_data_init(void);

#endif /* __VOIP_CALLS_H__ */

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * ex: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */


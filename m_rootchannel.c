/* Copyright (C) All Rights Reserved
** Written by Server
** Website: https://github.com/modmenager/unrealircd-modules/
** License: GPLv2 o successiva.Sei libero di modificarlo e adattarlo alle esigenze della tua rete IRC.
*/

/*
 * Blocco ASSOLUTO canale #root - Ammette SOLO root e root-with-override
 * Compatibile con UnrealIRCd 6.2.7
 */

/*** <<<MODULE MANAGER START>>>
module {
    documentation "https://github.com/modmenager/unrealircd-modules/blob/main/README.md";
    troubleshooting "In caso di problemi con il modulo m_rootchannel, Contatta Server Su Irc.albnetwork.org.";
    min-unrealircd-version "6.1.*";
    max-unrealircd-version "6.2.*";
    post-install-text {
        "Il modulo m_rootchannel e' stato installato con successo!";
        "Ora aggiungi la seguente riga nel tuo file unrealircd.conf:";
        "loadmodule \"third/m_rootchannel\";";
        "Ricordati di creare il file 'conf/rootchannel.txt' inserendo il nome del canale da blindare (es. #root).";
        "Infine, esegui /REHASH dal tuo client IRC per attivarlo.";
    }
}
*** <<<MODULE MANAGER END>>>
*/


#include "unrealircd.h"

ModuleHeader MOD_HEADER = {
    "third/m_rootchannel",
    "17.1",
    "Blocco ASSOLUTO canale #root - Ammette SOLO root e root-with-override",
    "Server",
    "unrealircd-6"
};

static char restricted_channel[CHANNELLEN + 1] = "#root";

/* Prototipi delle funzioni */
int rootchannel_can_join(Client *client, Channel *channel, const char *key, char **errmsg);
void load_rootchannel_config(void);

/* Funzioni di override separate per intercettare i comandi alla fonte */
void override_join(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[]);
void override_sajoin(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[]);
void override_samode(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[]);
void override_sapart(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[]);
void override_invite(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[]);

MOD_TEST()
{
    return MOD_SUCCESS;
}

MOD_INIT()
{
    load_rootchannel_config();
    /* Manteniamo l'hook del join per sicurezza del core */
    HookAdd(modinfo->handle, HOOKTYPE_CAN_JOIN, 90, rootchannel_can_join);
    return MOD_SUCCESS;
}

MOD_LOAD()
{
    /* 
     * Registriamo l'override anche sul comando JOIN standard.
     * Questo intercetta il /JOIN classico dell'utente prima di qualsiasi logica di OperOverride.
     */
    CommandOverrideAdd(modinfo->handle, "JOIN", 50, override_join);
    CommandOverrideAdd(modinfo->handle, "SAJOIN", 50, override_sajoin);
    CommandOverrideAdd(modinfo->handle, "SAMODE", 50, override_samode);
    CommandOverrideAdd(modinfo->handle, "SAPART", 50, override_sapart);
    CommandOverrideAdd(modinfo->handle, "INVITE", 50, override_invite);
    return MOD_SUCCESS;
}

MOD_UNLOAD()
{
    return MOD_SUCCESS;
}

void load_rootchannel_config(void)
{
    FILE *f = fopen("conf/rootchannel.txt", "r");
    if (f)
    {
        char buf[CHANNELLEN + 1];
        if (fgets(buf, sizeof(buf), f))
        {
            buf[strcspn(buf, "\r\n")] = 0;
            if (buf[0] == '#')
            {
                strlcpy(restricted_channel, buf, sizeof(restricted_channel));
            }
        }
        fclose(f);
    }
}

/* Funzione helper interna per determinare ed eseguire il blocco */
int ed_esegui_blocco(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[], const char *cmd_text, const char *target_chan)
{
    if (target_chan && restricted_channel[0] != '\0' && strcasecmp(target_chan, restricted_channel) == 0)
    {
        /* Consente l'azione unicamente se l'operatore e' esplicitamente un Root autorizzato */
        if (ValidatePermissionsForPath("operclass:root", client, NULL, NULL, NULL) ||
            ValidatePermissionsForPath("operclass:root-with-override", client, NULL, NULL, NULL))
        {
            CallCommandOverride(ovr, clictx, client, mtags, parc, parv);
            return 1;
        }

        /* Invio della notifica in albanese e distruzione immediata del comando */
        sendnotice(client, "*** [Aksesi i ndaluar] Kanali %s eshte i rezervuar vetem per Root Administrator. Komanda %s u bllokua.", restricted_channel, cmd_text);
        return 0; 
    }
    
    /* Se non riguarda il canale protetto, prosegui normalmente */
    CallCommandOverride(ovr, clictx, client, mtags, parc, parv);
    return 1;
}

/* 0. OVERRIDE JOIN STANDARD: /JOIN <#canale> */
void override_join(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[])
{
    /* Nel comando JOIN il canale è il primo parametro: parv */
    const char *target_chan = (parc >= 2 && parv && parv[1]) ? parv[1] : NULL;
    ed_esegui_blocco(ovr, clictx, client, mtags, parc, parv, "JOIN", target_chan);
}

/* 1. OVERRIDE SAJOIN: /SAJOIN <nick> <#canale> */
void override_sajoin(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[])
{
    /* Nel comando SAJOIN il canale è il secondo parametro: parv */
    const char *target_chan = (parc >= 3 && parv && parv[2]) ? parv[2] : NULL;
    ed_esegui_blocco(ovr, clictx, client, mtags, parc, parv, "SAJOIN", target_chan);
}

/* 2. OVERRIDE SAMODE: /SAMODE <#canale> <modi> */
void override_samode(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[])
{
    const char *target_chan = (parc >= 2 && parv && parv[1]) ? parv[1] : NULL;
    ed_esegui_blocco(ovr, clictx, client, mtags, parc, parv, "SAMODE", target_chan);
}

/* 3. OVERRIDE SAPART: /SAPART <nick> <#canale> */
void override_sapart(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[])
{
    const char *target_chan = (parc >= 3 && parv && parv[2]) ? parv[2] : NULL;
    ed_esegui_blocco(ovr, clictx, client, mtags, parc, parv, "SAPART", target_chan);
}

/* 4. OVERRIDE INVITE: /INVITE <nick> <#canale> */
void override_invite(CommandOverride *ovr, ClientContext *clictx, Client *client, MessageTag *mtags, int parc, const char *parv[])
{
    const char *target_chan = (parc >= 3 && parv && parv[2]) ? parv[2] : NULL;
    ed_esegui_blocco(ovr, clictx, client, mtags, parc, parv, "INVITE", target_chan);
}

/* Blocco di sicurezza secondario sul JOIN standard del core */
int rootchannel_can_join(Client *client, Channel *channel, const char *key, char **errmsg)
{
    if (!client || !channel || channel->name[0] == '\0' || restricted_channel[0] == '\0')
        return HOOK_CONTINUE;

    if (strcasecmp(channel->name, restricted_channel) == 0)
    {
        if (ValidatePermissionsForPath("operclass:root", client, NULL, NULL, NULL) ||
            ValidatePermissionsForPath("operclass:root-with-override", client, NULL, NULL, NULL))
        {
            return 0; 
        }

        sendnotice(client, "*** [Aksesi i ndaluar] Kanali %s eshte i rezervuar vetem per Root Administrator.", channel->name);
        sendnumeric(client, ERR_BANNEDFROMCHAN, channel->name);
        
        *errmsg = "Aksesi i ndaluar.";
        return ERR_OPERONLY; 
    }

    return HOOK_CONTINUE;
}

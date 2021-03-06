//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: CS's custom CPlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "cs_player.h"
#include "player_resource.h"
#include "cs_player_resource.h"
#include "weapon_c4.h"
#include <coordsize.h>
#include "cs_gamerules.h"

// Datatable
IMPLEMENT_SERVERCLASS_ST(CCSPlayerResource, DT_CSPlayerResource)
	SendPropInt( SENDINFO( m_iPlayerC4 ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iPlayerVIP ), 8, SPROP_UNSIGNED ),
	SendPropVector( SENDINFO(m_vecC4), -1, SPROP_COORD),
	SendPropArray3( SENDINFO_ARRAY3(m_bHostageAlive), SendPropInt( SENDINFO_ARRAY(m_bHostageAlive), 1, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_isHostageFollowingSomeone), SendPropInt( SENDINFO_ARRAY(m_isHostageFollowingSomeone), 1, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iHostageEntityIDs), SendPropInt( SENDINFO_ARRAY(m_iHostageEntityIDs), -1, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iHostageY), SendPropInt( SENDINFO_ARRAY(m_iHostageY), COORD_INTEGER_BITS+1, 0 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iHostageX), SendPropInt( SENDINFO_ARRAY(m_iHostageX), COORD_INTEGER_BITS+1, 0 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iHostageZ), SendPropInt( SENDINFO_ARRAY(m_iHostageZ), COORD_INTEGER_BITS+1, 0 ) ),
	SendPropVector( SENDINFO(m_bombsiteCenterA), -1, SPROP_COORD),
	SendPropVector( SENDINFO(m_bombsiteCenterB), -1, SPROP_COORD),
	SendPropArray3( SENDINFO_ARRAY3(m_hostageRescueX), SendPropInt( SENDINFO_ARRAY(m_hostageRescueX), COORD_INTEGER_BITS+1, 0 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_hostageRescueY), SendPropInt( SENDINFO_ARRAY(m_hostageRescueY), COORD_INTEGER_BITS+1, 0 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_hostageRescueZ), SendPropInt( SENDINFO_ARRAY(m_hostageRescueZ), COORD_INTEGER_BITS+1, 0 ) ),
	SendPropBool( SENDINFO( m_bBombSpotted ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_bPlayerSpotted), SendPropInt( SENDINFO_ARRAY(m_bPlayerSpotted), 1, SPROP_UNSIGNED ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CCSPlayerResource )
	// DEFINE_ARRAY( m_iPing, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iPacketloss, FIELD_INTEGER, MAX_PLAYERS+1 ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( cs_player_manager, CCSPlayerResource );

CCSPlayerResource::CCSPlayerResource( void )
{
	
}


//--------------------------------------------------------------------------------------------------------
class Spotter
{
public:
	Spotter( CBaseEntity *entity, const Vector &target, int spottingTeam )
	{
		m_targetEntity = entity;
		m_target = target;
		m_team = spottingTeam;
		m_spotted = false;
	}

	bool operator()( CBasePlayer *player )
	{
		if ( !player->IsAlive() || player->GetTeamNumber() != m_team )
			return true;

		CCSPlayer *csPlayer = ToCSPlayer( player );
		if ( !csPlayer )
			return true;

		if ( csPlayer->IsBlind() )
			return true;

		Vector eye, forward;
		player->EyePositionAndVectors( &eye, &forward, NULL, NULL );
		Vector path( m_target - eye );
		float distance = path.Length();
		path.NormalizeInPlace();
		float dot = DotProduct( forward, path );
		if( (dot > 0.995f ) 
			|| (dot > 0.98f && distance < 900) 
			|| (dot > 0.8f && distance < 250) 
			)
		{
			trace_t tr;
			CTraceFilterSkipTwoEntities filter( player, m_targetEntity, COLLISION_GROUP_DEBRIS );
			UTIL_TraceLine( eye, m_target,
				(CONTENTS_OPAQUE|CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_DEBRIS), &filter, &tr );

			if( tr.fraction == 1.0f )
			{
				m_spotted = true;
				return false; // spotted already, so no reason to check for other players spotting the same thing.
			}
		}

		return true;
	}

	bool Spotted( void ) const
	{
		return m_spotted;
	}

private:
	CBaseEntity *m_targetEntity;
	Vector m_target;
	int m_team;
	bool m_spotted;
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CCSPlayerResource::UpdatePlayerData( void )
{
	int i;

	m_iPlayerC4 = 0;
	m_iPlayerVIP = 0;

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CCSPlayer *pPlayer = (CCSPlayer*)UTIL_PlayerByIndex( i );
		
		if ( pPlayer && pPlayer->IsConnected() )
		{
			if ( pPlayer->IsVIP() )
			{
				// we should only have one VIP
				Assert( m_iPlayerVIP == 0 );
				m_iPlayerVIP = i;
			}

			if ( pPlayer->HasC4() )
			{
				// we should only have one bomb
				m_iPlayerC4 = i;
			}
		}
	}

	CBaseEntity *c4 = NULL;
	if ( m_iPlayerC4 == 0 )
	{
		// no player has C4, update C4 position
		if ( g_C4s.Count() > 0 )
		{
			c4 = g_C4s[0];
			m_vecC4 = c4->GetAbsOrigin();
		}
		else
		{
			m_vecC4.Init();
		}
	}


	bool bombSpotted = false;
	if ( c4 )
	{
		Spotter spotter( c4, m_vecC4, TEAM_CT );
		ForEachPlayer( spotter );
		if ( spotter.Spotted() )
		{
			bombSpotted = true;
		}
	}

	for ( int i=0; i < MAX_PLAYERS+1; i++ )
	{
		CCSPlayer *target = ToCSPlayer( UTIL_PlayerByIndex( i ) );
		if ( !target || !target->IsAlive() )
		{
			m_bPlayerSpotted.Set( i, 0 );
			continue;
		}

		Spotter spotter( target, target->EyePosition(), (target->GetTeamNumber()==TEAM_CT) ? TEAM_TERRORIST : TEAM_CT );
		ForEachPlayer( spotter );
		if ( spotter.Spotted() )
		{
			if ( target->HasC4() )
			{
				bombSpotted = true;
			}
			m_bPlayerSpotted.Set( i, 1 );
		}
		else
		{
			m_bPlayerSpotted.Set( i, 0 );
		}
	}

	if ( bombSpotted )
	{
		m_bBombSpotted = true;
	}
	else
	{
		m_bBombSpotted = false;
	}

	BaseClass::UpdatePlayerData();
}

void CCSPlayerResource::Spawn( void )
{
	m_vecC4.Init();
	m_iPlayerC4 = 0;
	m_iPlayerVIP = 0;
	m_bombsiteCenterA.Init();
	m_bombsiteCenterB.Init();
	m_foundGoalPositions = false;

	for ( int i=0; i < MAX_HOSTAGES; i++ )
	{
		m_bHostageAlive.Set( i, 0 );
		m_isHostageFollowingSomeone.Set( i, 0 );
		m_iHostageEntityIDs.Set(i, 0);
	}

	for ( int i=0; i < MAX_HOSTAGE_RESCUES; i++ )
	{
		m_hostageRescueX.Set( i, 0 );
		m_hostageRescueY.Set( i, 0 );
		m_hostageRescueZ.Set( i, 0 );
	}

	m_bBombSpotted = false;
	for ( int i=0; i < MAX_PLAYERS+1; i++ )
	{
		m_bPlayerSpotted.Set( i, 0 );
	}

	BaseClass::Spawn();
}

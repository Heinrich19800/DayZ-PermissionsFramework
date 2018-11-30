class PermissionManager
{
    ref array< ref AuthPlayer > AuthPlayers;

    ref Permission RootPermission;

    void PermissionManager()
    {
        AuthPlayers = new ref array< ref AuthPlayer >;

        RootPermission = new ref Permission( "ROOT", NULL );
    }

    array< ref AuthPlayer > GetPlayers( ref array< string > guids = NULL )
    {
        if ( guids == NULL )
        {
            return AuthPlayers;
        }

        array< ref AuthPlayer > tempArray = new array< ref AuthPlayer >;

        for ( int i = 0; i < guids.Count(); i++ )
        {
            for ( int k = 0; k < AuthPlayers.Count(); k++ )
            {
                if ( guids[i] == AuthPlayers[k].GetGUID() )
                {
                    tempArray.Insert( AuthPlayers[k] );
                }
            }
        }

        return tempArray;
    }

    void SetPlayers( ref array< ref AuthPlayer > players )
    {
        AuthPlayers.Clear();

        // This doesn't work??? wtf
        //AuthPlayers.Copy( players );

        for ( int i = 0; i < players.Count(); i++ )
        {
            AuthPlayers.Insert( players[i] );
        }
    }

    void AddPlayers( ref array< ref AuthPlayer > players )
    {
        for ( int i = 0; i < players.Count(); i++ )
        {
            AuthPlayers.Insert( players[i] );
        }
    }

    void RegisterPermission( string permission )
    {
        RootPermission.AddPermission( permission, PermissionType.INHERIT );
    }

    ref array< string > Serialize()
    {
        ref array< string > data = new ref array< string >;
        RootPermission.Serialize( data );
        return data;
    }

    ref Permission GetRootPermission()
    {
        return RootPermission;
    }

    bool HasPermission( string permission, PlayerIdentity player = NULL )
    {
        if ( !GetGame().IsMultiplayer() ) return true;

        if ( player == NULL ) 
        {
            if ( ClientAuthPlayer == NULL )
            {
                return true;
            }

            return ClientAuthPlayer.HasPermission( permission );
        } 

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GetGUID() == player.GetId() )
            {
                return AuthPlayers[i].HasPermission( permission );
            }
        }

        return false;
    }

    ref AuthPlayer PlayerJoined( PlayerIdentity player )
    {
        ref PlayerData data = new ref PlayerData;

        if ( player )
        {
            data.SName = player.GetName();
            data.SGUID = player.GetId();
        } else 
        {
            data.SName = "Offline Mode";
            data.SGUID = "N/A";
        }

        ref AuthPlayer auPlayer = new ref AuthPlayer( data );

        if ( player )
        {
            auPlayer.SetIdentity( player );
        }

        auPlayer.CopyPermissions( RootPermission );

        auPlayer.Load();
        
        AuthPlayers.Insert( auPlayer );

        return auPlayer;
    }

    void PlayerLeft( PlayerIdentity player )
    {
        if ( player == NULL ) return;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            ref AuthPlayer auPlayer = AuthPlayers[i];
            
            if ( auPlayer.GetGUID() == player.GetId() )
            {
                auPlayer.Save();

                if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
                {
                    GetRPCManager().SendRPC( "PermissionsFramework", "RemovePlayer", new Param1< ref PlayerData >( SerializePlayer( auPlayer ) ), true );
                }

                AuthPlayers.Remove( i );
                break;
            }
        }
    }

    void DebugPrint()
    {
        Print( "Printing all authenticated players!" );
        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            AuthPlayers[i].DebugPrint();
        }
    }

    ref AuthPlayer GetPlayerByGUID( string guid )
    {
        ref AuthPlayer auPlayer = NULL;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GetGUID() == guid )
            {
                auPlayer = AuthPlayers[i];
                break;
            }
        }

        if ( auPlayer == NULL )
        {
            ref PlayerData data = new ref PlayerData;
            
            data.SGUID = guid;

            auPlayer = new ref AuthPlayer( data );

            AuthPlayers.Insert( auPlayer );
        }

        return auPlayer;
    }

    ref AuthPlayer GetPlayerByIdentity( PlayerIdentity ident )
    {
        if ( ident == NULL ) return NULL;

        ref AuthPlayer auPlayer = NULL;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GetGUID() == ident.GetId() )
            {
                auPlayer = AuthPlayers[i];
                break;
            }
        }

        if ( auPlayer == NULL )
        {
            auPlayer = PlayerJoined( ident );
        }

        return auPlayer
    }

    ref AuthPlayer GetPlayer( ref PlayerData data )
    {
        if ( data == NULL ) return NULL;
        
        ref AuthPlayer auPlayer = NULL;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GetGUID() == data.SGUID )
            {
                auPlayer = AuthPlayers[i];
                break;
            }
        }

        if ( auPlayer == NULL )
        {
            auPlayer = new ref AuthPlayer( data );

            AuthPlayers.Insert( auPlayer );
        }

        auPlayer.SwapData( data );

        auPlayer.Deserialize();

        return auPlayer;
    }
}

ref PermissionManager g_com_PermissionsManager;

ref PermissionManager GetPermissionsManager()
{
    if( !g_com_PermissionsManager )
    {
        g_com_PermissionsManager = new ref PermissionManager();
    }

    return g_com_PermissionsManager;
}
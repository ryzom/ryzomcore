. $RIBS_PATH/utils.sh

SHARD_PATH="/home/nevrax/shard"
BUILD_PATH="/home/nevrax/builds"
PATCH_PATH="/home/nevrax/patchs"
RYZOMCORE_PATH="/home/nevrax/repos/ryzom-core"
RYZOMDATA_PATH="/home/nevrax/repos/ryzom-data"
RYZOMSERVER_PATH="/home/nevrax/repos/ryzom-core/ryzom/server"
RYZOMSERVERDATA_PATH="/home/nevrax/repos/ryzom-private-data"
RYZOMEXTERNALS_PATH="/home/nevrax/repos/ryzom-externals"
SERVER_NEL_PATH=""
WEB_PATH="/home/nevrax/www"
SHARD_NAME=$(get_ini_var shard name /etc/ryzom/shard.ini)
SHARD_HOST=${SHARD_NAME^}
SHARD_DESC=$(get_ini_var shard desc /etc/ryzom/shard.ini)
SHARD_ID=$(get_ini_var shard id /etc/ryzom/shard.ini)
SHARD_DOMAIN=$(get_ini_var shard domain /etc/ryzom/shard.ini)
SHARD_IS_DEV=$(get_ini_var shard name /etc/ryzom/shard.ini)
if [ "$SHARD_IS_DEV" == "1" ]; then
SHARD_TYPE="test"
else
SHARD_TYPE="live"
fi
RELEASENOTE_TOKEN="xxx"
DB_HOST=$(get_ini_var db_ring host /etc/ryzom/shard.ini)
DB_RING=$(get_ini_var db_ring name /etc/ryzom/shard.ini)
DB_USER=$(get_ini_var db_ring user /etc/ryzom/shard.ini)
DB_PASS=$(get_ini_var db_ring pass /etc/ryzom/shard.ini)
SALT="xxx"
SHARD_LIVE_DOMAINS=""

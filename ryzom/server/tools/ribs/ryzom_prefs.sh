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
SHARD_NAME="gingo"
SHARD_HOST="Gingo"
SHARD_DESC="(Test Shard)"
SHARD_ID="501"
SHARD_DOMAIN="ryzom_test"
SHARD_TYPE="test"
WEB_URL="https://Gingo"
RELEASENOTE_TOKEN="x8dslpOKksdookd972930kdjzpd0897e9sicd293dokdoeo"
DB_HOST=$(get_ini_var db_ring host /etc/ryzom/shard.ini)
DB_RING=$(get_ini_var db_ring name /etc/ryzom/shard.ini)
DB_USER=$(get_ini_var db_ring user /etc/ryzom/shard.ini)
DB_PASS=$(get_ini_var db_ring pass /etc/ryzom/shard.ini)
SALT="CrxZeSNmzsOrp6SpqphVGWOxD16wwGaJ"
SHARD_LIVE_DOMAINS=""

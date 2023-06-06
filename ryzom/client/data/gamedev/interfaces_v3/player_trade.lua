-- In this file we define functions that serves for player windows
local base = "ui:interface:player_trade:header_opened";
local slots = 10; -- Change in local_database.xml, database.xml and NbExchangeSlots on server

------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

------------------------------------------------------------------------------------------------------------
-- Update weight and bulk indicator in trade window for receive side
function game:updateReceiveBulkAndWeight()
	getUI(base .. ":receive:bulk_txt").hardtext = tostring(getBulk("RECEIVE"));
	getUI(base .. ":receive:weight_txt").hardtext = tostring(getWeight("RECEIVE"));
end

-- Update weight and bulk indicator in trade window for give side
function game:updateGiveBulkAndWeight()
	getUI(base .. ":give:bulk_txt").hardtext = tostring(getBulk("GIVE"));
	getUI(base .. ":give:weight_txt").hardtext = tostring(getWeight("GIVE"));
end

function getWeight(inventory)
	local weight = runExpr("getItemsWeight('LOCAL:EXCHANGE:"..inventory.."', 0," .. slots .. ")");
	return math.floor(weight * 100) / 100;
end

function getBulk(inventory)
	local bulk = runExpr("getItemsBulk('LOCAL:EXCHANGE:"..inventory.."', 0," .. slots .. ")");
	return math.floor(bulk * 100) / 100;
end

-- VERSION --
RYZOM_PLAYER_TRADE_VERSION = 10469
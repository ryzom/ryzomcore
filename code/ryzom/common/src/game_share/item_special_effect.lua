require("enum_generator")

name = { "item", "special", "effect" }
enum = {
	"ISE_FIGHT_ADD_CRITICAL",			-- crit % (float)
	"ISE_FIGHT_VAMPIRISM",				-- proc % (float)
	"ISE_MAGIC_DIVINE_INTERVENTION",	-- proc % (float)
	"ISE_MAGIC_SHOOT_AGAIN",			-- proc % (float), maximum bonus duration (seconds) (float)
	"ISE_CRAFT_ADD_STAT_BONUS",			-- proc % (float), bonus value (float)
	"ISE_CRAFT_ADD_LIMIT",				-- proc % (float), bonus value % (float)
	"ISE_FORAGE_ADD_RM",				-- proc % (float), bonus value % (float)
	"ISE_FORAGE_NO_RISK",				-- proc % (float)
}

enum_generator.build(name, enum)

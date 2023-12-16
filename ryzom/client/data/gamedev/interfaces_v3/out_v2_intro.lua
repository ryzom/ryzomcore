function get_occ_events()
	local application = getClientCfgVar("Application")
	getUI("ui:outgame:connecting:html"):renderHtml([[
		<object type="application/ryzom-data" standby="override" data="https://]]..getClientCfgVar("WebIgMainDomain")..[[/data/]]..tostring(application["0"])..[[/enable_events_bnp.csv"></object>
		<object type="application/ryzom-data" standby="override" data="https://]]..getClientCfgVar("WebIgMainDomain")..[[/data/]]..tostring(application["0"])..[[/enable_occs_bnp.csv"></object>
	]])
end

-- VERSION --
RYZOM_OUT_V2_INTRO_VERSION = 341

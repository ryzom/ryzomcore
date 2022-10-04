--
-- teleport interface
--
local html = [[
<html>
    <head>
        <meta http-equiv="content-type" content="text/html;charset=utf-8" />
    </head>
    <body style="background-image:url(skin_blank.tga);">
        <table cellspacing="3" cellpadding="3" width="100%">
            <div name="info_banner">
                <tr><img src="#faction.tga"></tr>
            </div>
            <table cellspacing="#csp" cellpadding="2" width="100%">
                <tr>
                    <td>
                        #fyros
                    </td>
                </tr>
            </table>
            <table cellspacing="#csp" cellpadding="2" width="100%">
                <tr>
                    <td>
                        #tryker
                    </td>
                </tr>
            </table>
            <table cellspacing="#csp" cellpadding="2" width="100%">
                <tr>
                    <td>
                        #matis
                    </td>
                </tr>
            </table>
            <table cellspacing="#csp" cellpadding="2" width="100%">
                <tr>
                    <td>
                        #zorai
                    </td>
                </tr>
            </table>
            <table cellspacing="#csp" cellpadding="2" width="100%">
                <tr>
                    <td>
                        #primes
                    </td>
                </tr>
            </table>
            <div name="info_footer">
                <table width="100%">
                    <tr>
                        <div name="info_money">
                            <td><img align="left" src="money_seve.tga"></td>
                            <td width="100%"><font color="white" size="12">#money</font></td>
                        </div>
                        <td><font color="white" size="10">#dopact</font></td>
                        <td><form><input name="auto" alt="#help" type="checkbox" #check></form></td>
                    </tr>
                </table>
            </div>
        </table>
    </body>
</html>]]

if artefact == nil then
    artefact = {
        h = 479,
        w = 526,
        max_h = 479,
        max_w = 603,
        min_h = 200,
        min_w = 234,
        cellspace = 5,
        isCompact = false,
        isAttached = false,
        isFiltered = false,
        isMinimized = true,
        isLogoMinimized = false,
        web_item = "params_l:#L;template:webig_inv_item_artefact;\
                    display:inline-block;id:pact#P;tooltip:u:#T;\
                    quantity:q0;#Q;#O;img1:tp_#I.tga;\
                    bg:bk_#E.tga;col_over:255 255 255 45", -- slotbg:blank2.tga;
        uiWindow = nil,
        uiWindowBag = nil,
        idWindow = "ui:interface:artefact",
        idWindowBag = "ui:interface:inventory:content:bag",
        webcode = html,
        zone = "",
        items_labels = Json.decode([[{"kami_almati":"almati","kami_avalae":"avalae","kami_avendale":"avendale","kami_bountybeaches":"bounty","kami_citiesofintuition":"","kami_corrupted_moor":"","kami_corrupted_moor_fairhaven":"","kami_corrupted_moor_pyr":"","kami_corrupted_moor_yrkanis":"","kami_corrupted_moor_zora":"","kami_crystabell":"crystabell","kami_davae":"davae","kami_dewdrops":"dewdrops","kami_dunesofexil":"dunes","kami_dyron":"dyron","kami_enchantedisle":"enchanted","kami_fairhaven":"fairhaven","kami_fleetinggarden":"fleeting","kami_forbidden_depths":"forbidden","kami_frahartowers":"frahar","kami_gate_of_obscurity":"gate","kami_groveofconfusion":"grove","kami_groveofumbra":"grove","kami_havenofpurity":"haven","kami_hereticshovel":"heretic","kami_hiddensource":"hidden","kami_hoi_cho":"hoicho","kami_jen_lai":"jenlai","kami_knollofdissent":"knoll","kami_knotofdementia":"knot","kami_lagoonsofloria":"lagoon","kami_maidengrove":"maiden","kami_min_cho":"mincho","kami_natae":"natae","kami_nexus_bagne":"","kami_nexus_route_gouffre":"","kami_nexus_terre":"nexus","kami_oflovaksoasis":"oflovak","kami_outlawcanyon":"outlaw","kami_pyr":"pyr","kami_ranger_camp":"ranger","kami_restingwater":"resting","kami_sawdustmines":"sawdust","kami_shining_lake":"shining","kami_the_abyss_of_ichor_matis":"abyss","kami_the_abyss_of_ichor_nexus":"","kami_the_elusive_forest":"elusive","kami_the_land_of_continuity":"land","kami_the_sunken_city":"sunken","kami_the_trench_of_trials_tryker":"","kami_the_trench_of_trials_zorai":"trench","kami_the_under_spring_fyros":"under","kami_the_under_spring_zorai":"","kami_the_windy_gate":"windy","kami_thefount":"fount","kami_thesavagedunes":"savage","kami_thescorchedcorridor":"scorched","kami_thesos":"thesos","kami_thevoid":"void","kami_upperbog":"upper","kami_windermeer":"windermeer","kami_windsofmuse":"winds","kami_yrkanis":"yrkanis","kami_zora":"zora","kami_zorai_nland":"","karavan_almati":"almati","karavan_avalae":"avalae","karavan_avendale":"avendale","karavan_bountybeaches":"bounty","karavan_corrupted_moor":"","karavan_corrupted_moor_fairhaven":"","karavan_corrupted_moor_pyr":"","karavan_corrupted_moor_yrkanis":"","karavan_corrupted_moor_zora":"","karavan_crystabell":"crystabell","karavan_davae":"davae","karavan_dewdrops":"dewdrops","karavan_dunesofexil":"dunes","karavan_dyron":"dyron","karavan_enchantedisle":"enchanted","karavan_fairhaven":"fairhaven","karavan_fleetinggarden":"fleeting","karavan_forbidden_depths":"forbidden","karavan_frahartowers":"frahar","karavan_gate_of_obscurity":"gate","karavan_groveofconfusion":"grove","karavan_groveofumbra":"grove","karavan_havenofpurity":"haven","karavan_hereticshovel":"heretic","karavan_hiddensource":"hidden","karavan_hoi_cho":"hoicho","karavan_jen_lai":"jenlai","karavan_knollofdissent":"knoll","karavan_knotofdementia":"knot","karavan_lagoonsofloria":"lagoon","karavan_maidengrove":"maiden","karavan_majesticgarden":"","karavan_matis_nland":"","karavan_min_cho":"mincho","karavan_natae":"natae","karavan_nexus_bagne":"","karavan_nexus_route_gouffre":"","karavan_nexus_terre":"nexus","karavan_oflovaksoasis":"oflovak","karavan_outlawcanyon":"outlaw","karavan_pyr":"pyr","karavan_ranger_camp":"ranger","karavan_restingwater":"resting","karavan_sawdustmines":"sawdust","karavan_shattered_ruins":"shattered","karavan_the_abyss_of_ichor_matis":"abyss","karavan_the_abyss_of_ichor_nexus":"","karavan_the_elusive_forest":"elusive","karavan_the_land_of_continuity":"land","karavan_the_sunken_city":"sunken","karavan_the_trench_of_trials_tryker":"","karavan_the_trench_of_trials_zorai":"trench","karavan_the_under_spring_fyros":"under","karavan_the_under_spring_zorai":"","karavan_the_windy_gate":"windy","karavan_thefount":"fount","karavan_thesavagedunes":"savage","karavan_thescorchedcorridor":"scorched","karavan_thesos":"thesos","karavan_thevoid":"void","karavan_upperbog":"upper","karavan_windermeer":"windermeer","karavan_windsofmuse":"winds","karavan_yrkanis":"yrkanis","karavan_zora":"zora"}]])
    }

    -- on_event window
    function artefact:onActive()
        if self.uiWindow then
            self:restoreWindow()
            self:bagObserver(true)
        end
    end

    function artefact:onDeactive()
        if self.uiWindow then
            self.uiWindow.active = false
            self.uiWindow.opened = false
            if self.observer then
                self:bagObserver(false)
            end
        end
    end

    function artefact:onClose()
        if self.uiWindow then
            runAH(getUICaller(), "proc", "artefact_proc_deactive")
            self:restorePact()
        end
    end

    function artefact:onClickHeader(opened)
        if self.uiWindow then
            if opened > 0 then
                self:restoreWindow()
                self.uiWindow:find("header_minimize").params_l = "artefact:onClickHeader(0)"
                return
            end
            -- on_close
            self:saveWindow()
            self.uiWindow:find("header_minimize").params_l = "artefact:onClickHeader(1)"
        end
    end

    function artefact:onResize()
        -- on resize max width
        if self.uiWindow.w == self.max_w then
            local size = 42
            -- adjust height size
            if getDbProp(self.banner) == 0 then
                size = 112
            end
            self.uiWindow.pop_max_h = self.max_h - size
            -- vertical down only
            if self.uiWindow.h >= self.uiWindow.pop_max_h then
                -- now resize window height
                self.uiWindow.h = self.uiWindow.pop_max_h
            end
        else
            if not self.isCompact then
                if getDbProp(self.banner) == 0 then
                    self.uiWindow.pop_max_h = self.max_h - 69
                    -- on resize height if width not maxed
                    if self.uiWindow.h >= self.uiWindow.pop_max_h then
                        self.uiWindow.h = self.uiWindow.pop_max_h
                    end
                else
                    -- logo pop up
                    self.uiWindow.pop_max_h = self.max_h
                end
            end
        end
        -- force dimension
        if self.isMinimized then
            self.uiWindow.h = self.uiWindow.pop_min_h
            self.uiWindow.w = self.uiWindow.pop_min_w
        end
        -- special case
        if self.isCompact then
            if not self.isMinimized then
                self.uiWindow.pop_max_w = self.min_h/2 + 1
                self.uiWindow.pop_max_h = getUI("ui:interface").h
            end
        end
    end

    -- toggle on checkbox event
    function artefact:onChecked()
        if getDbProp(self.dopact) == 1 then
            setDbProp(self.dopact, 0)
            sendMsgToServerAutoPact(false)
            return
        end
        setDbProp(self.dopact, 1)
        sendMsgToServerAutoPact(true)
    end

    function artefact:onMenu(menu)
        local node = ":toggle_banner_"
        -- can not switch logo in compact mode
        if self.isCompact then
            getUI(menu..node.."hide").active = false
            getUI(menu..node.."show").active = false
        else
            if getDbProp(self.banner) == 1 then
                getUI(menu..node.."hide").active = true
                getUI(menu..node.."show").active = false
            else
                getUI(menu..node.."hide").active = false
                getUI(menu..node.."show").active = true
            end
        end
        node = ":window_on_tp_"
        -- toggle on teleport close menu
        if getDbProp(self.closeTp) == 1 then
            getUI(menu..node.."close").active = false
            getUI(menu..node.."open").active = true
        else
            getUI(menu..node.."close").active = true
            getUI(menu..node.."open").active = false
        end
        node = ":window_compact"
        -- disable compact mode if minimized
        if self.isMinimized then
            getUI(menu..node).active = false
        else
            getUI(menu..node).active = true
        end
        runAH(getUICaller(), "active_menu", "menu="..menu)
    end

    -- event onclick menu
    function artefact:onSelect(event)
        -- toggle on teleport close
        if event == 2 then
            if getDbProp(self.closeTp) == 1 then
                setDbProp(self.closeTp, 0)
                return
            end
            setDbProp(self.closeTp, 1)
            return
        end
        -- do not switch mode while minimized
        if self.isMinimized and event == 4 then
            return
        end
        -- do not allow logo in compact mode
        if event == 1 then
            if self.isCompact then
                return
            end
            -- toggle logo
            if getDbProp(self.banner) == 1 then
                setDbProp(self.banner, 0)
                self.uiWindow.h = self.max_h - 69
            else
                setDbProp(self.banner, 1)
                self.uiWindow.pop_max_h = self.max_h
                self.uiWindow.h = self.max_h
                -- draw in minimized
                if self.isMinimized then
                    self.isLogoMinimized = true
                end
            end
        end
        -- toggle mode
        if event == 4 then
            if not self.isCompact then
                -- store original window
                self.pop_compact_h = self.uiWindow.h
                self.pop_compact_w = self.uiWindow.w
                self.pop_compact_l = getDbProp(self.banner)
                -- switch to compact
                setDbProp(self.banner, 0)
                self.isCompact = true
            else
                self.isCompact = false
                -- restore default mode
                self.uiWindow.h = self.pop_compact_h
                self.uiWindow.w = self.pop_compact_w
                self.uiWindow.pop_max_w = self.max_w
                if self.pop_compact_l == 1 then
                    setDbProp(self.banner, 1)
                end
            end
        end
        -- attach it to inventory bag
        if event == 5 then
            runAH(getUICaller(), "proc", "artefact_win_attach")
            return
        end
        self:doRefresh() -- event 3
        if event == 1 then
            self:onResize()
        end
    end

    function artefact:saveWindow()
        -- save current window dimension
        self.isMinimized = true
        self.h = self.uiWindow.h
        self.w = self.uiWindow.w
        -- minimize
        self.uiWindow.pop_min_h = 32
        self.uiWindow.pop_min_w = self.min_h / 2
        self.uiWindow.w = self.min_h / 2
        self.uiWindow.h = 0
    end

    function artefact:restoreWindow()
        if self.isMinimized then
            self.isMinimized = false
            -- restore dimension
            self.uiWindow.pop_min_h = self.min_h
            self.uiWindow.pop_min_w = self.min_w
            self.uiWindow.h = self.h
            self.uiWindow.w = self.w
            -- logo is activated when minimized
            if self.isLogoMinimized then
                self.uiWindow.h = self.uiWindow.pop_max_h
                -- reset
                self.isLogoMinimized = false
            end
            self:onResize()
        end
    end

    function artefact:attachWindow()
        if self.uiWindow.opened then
            self.uiWindow.opened = false
        end
        if self.uiWindow.active then
            self.uiWindow.active = false
        end
        self.isAttached = true
        -- render content in bag
        self.uiWindowBag = getUI(self.idWindowBag..":artefact_content")
        if self.uiWindowBag then
            self:doRefresh()
        end
        if not self.observer then
            self:bagObserver(true)
        end
    end

    function artefact:detachWindow()
        if self.observer then
            self:bagObserver(false)
        end
        self.isAttached = false
        self.uiWindow.opened = true
        self.uiWindow.active = true
        self.uiWindowBag = nil
        self:doRefresh()
    end

    -- hide inventory bag pacts
    function artefact:hidePact()
        if getDbProp(self.filter) == 1 then
            self.isFiltered = true
            setDbProp(self.filter, 0)
        end
    end

    -- restore inventory bag pacts
    function artefact:restorePact()
        if self.isFiltered then
            if getDbProp(self.active) == 0 then
                self.isFiltered = false
                setDbProp(self.filter, 1)
            end
        end
    end

    function artefact:doRefresh()
        -- faction change
        if not artefact:checkfame() then
            if artefact.observer then
                artefact:bagObserver(false)
            end
            if self.isAttached then
                runAH(getUICaller(), "proc", "artefact_win_detach")
            else
                artefact:startInterface()
            end
            return
        end
        local html = artefact.uiWindow:find("html")
        if artefact.uiWindowBag and artefact.isAttached then
            html = artefact.uiWindowBag:find("html")
        end
        if html then
            artefact:dynRender(html)
        end
    end

    function artefact:usePact(id)
        if not artefact:checkfame() then
            if self.isAttached then
                runAH(getUICaller(), "proc", "artefact_win_detach")
            else
                self:doRefresh()
            end
            return
        end
        sendMsgToServerUseItem(id)
        -- on teleport event
        if getDbProp(self.closeTp) == 1 then
            if self.isAttached then
                runAH(getUICaller(), "proc", "select_bag_items")
            end
            self:onClose()
        end
    end

    -- update interface
    function artefact:bagObserver(listen)
        if not self.observer then
            self.observer = misc:initInvObserver(
                self.bag,
                self.doRefresh, -- on_add
                self.doRefresh, -- on_del
                self.doRefresh, -- on_change
                "tp_"..self.faction
            )
        end
        -- attach observer
        local window = self.uiWindow
        if self.isAttached then
            if self.uiWindowBag then
                window = self.uiWindowBag
            end
        end
        if listen then
            self.observer:add(window, 1)
        else
            if self.observer then
                self.observer:remove(window, 1)
                self.observer = nil
            end
        end
    end

    function artefact:dynRender(html)
        local content = self.webcode
        -- list teleportation pacts
        for eco, tp in pairs(self.currentPacts) do
            for _, item in pairs(tp) do
                -- list items in bag
                for i = 0, self.max_slots-1 do
                    local sheet = self:getitem(i, "sheet")
                    -- sheet do exist
                    if sheet > 0 then
                        local name = getSheetName(sheet)
                        local quantity = self:getitem(i, "quantity")
                        -- matches with owned pacts
                        if self:strcmp(name, "tp_"..self.faction) then
                            if self:strcmp(name, ".sitem", false) then
                                if name:find(item.name) then
                                    self.zone = self:webInv(item, eco, quantity, i)
                                end
                            end
                        end
                    end
                end
            end
            content = string.gsub(content, "#"..eco, self.zone)
            self.zone = ""
        end
        local banner = "ban_artefact_"
        local dappers = getDbProp(self.dapper)
        local checkbox = getDbProp(self.dopact)
        local autopact = "Auto&nbsp;Pacts"
        -- reset table cellspace
        if self.cellspace > 5 then
            self.cellspace = 5
        end
        -- reset cellspace
        local space = self.cellspace
        -- mode
        if self.isCompact then
            autopact = "Pacts&nbsp;"
            -- center pacts
            space = space + 7
        end
        content = string.gsub(content, "#csp", space)
        -- toggle auto pact checkbox
        if checkbox > 0 then
            checkbox = "checked" else checkbox = ""
        end
        if dappers < 0 then dappers = 0 end
        -- construct page
        for k, v in pairs({
            faction = banner..self.faction,
            dopact = autopact,
            check = checkbox,
            money = self:formatint(dappers),
            help = i18n.get("uiArtefactHelp"):toUtf8()
        }) do
            content = string.gsub(content, "#"..k, v)
        end
        if content then
            -- now render
            html:renderHtml(content)
            -- set mode
            if self.isCompact then
                html:showDiv("info_money", false)
            end
            -- attach to bag
            if self.isAttached then
                local uiWindowBag = getUI(self.idWindowBag)
                if uiWindowBag then
                    uiWindowBag:find("autopact_vt").hardtext = "Auto Pact"
                end
                html:showDiv("info_banner", false)
                html:showDiv("info_footer", false)
                return
            end
            -- hide logo
            if getDbProp(self.banner) == 0 then
                html:showDiv("info_banner", false)
                self:onResize()
            end
            -- trigger checkbox event
            self.uiWindow:find("auto").onclick_l = "lua:artefact:onChecked()"
        end
    end

    -- use template
    function artefact:webInv(item, ecosystem, quantity, id)
        -- limit max quantity
        if quantity > 999 then
            quantity = 999
        end
        local n = 1
        local s = "q0:x;"
        -- format quantity as q1:;q2:;q3:;..
        for q in string.gmatch(tostring(quantity), "%d") do
            s = s.."q"..n..":"..q..";"
            n = n + 1
        end
        -- format overlay label as o1:;o2:;o3:;..
        for i = 1, math.min(8, #item.label) do
            s = s.."o"..tostring(i-1)..":"..item.label:sub(i, i)..";"
            n = n + 1
        end

        local wi = string.gsub(self.web_item, "#Q", s)
        local desc = item.desc:toUtf8()
        -- keep only the name of the zone
        for i = 1, #self.blacklist do
            desc = string.gsub(desc, self.blacklist[i], "")
        end

        desc = desc:gsub("^%l", string.upper)
        -- construct item
        for k, v in pairs({
            T = desc.." - "..self:formatint(item.cost),
            L = "artefact:usePact("..id..")",
            I = self.faction,
            E = ecosystem,
            P = id
        }) do
            wi = string.gsub(wi, "#"..k, v)
        end
        -- because ryzom
        if self.faction:find("kar") then
            wi = string.gsub(wi, "karavan", "caravane")
        end
        return self.zone..string.gsub(
            '<div class="ryzom-ui-grouptemplate" id="icon" style="#webitem"></div>',
            "#webitem", wi
        )
    end

    function artefact:strcmp(str, offset, s)
        if s ~= nil then
            return offset == '' or str:sub(-#offset) == offset
        end
        return str:sub(1, #offset) == offset
    end

    function artefact:formatint(n)
        local left, num, right = string.match(n, "^([^%d]*%d)(%d*)(.-)$")
        return left..(num:reverse():gsub("(%d%d%d)", "%1,"):reverse())..right
    end

    function artefact:getitem(id, s)
        if id ~= nil then
            return getDbProp(self.bag..':'..id..':'..s:upper())
        end
    end

    function artefact:getfaction(id)
        if not id then id = getDbProp(self.fame) end
        if id == 3 then return "karavan" end
        if id == 2 then return "kami" end
    end

    function artefact:checkRestriction(cult)
        local fame = getDbProp(self.fame)
        if fame > 1 and fame < 4 then
            if cult then
                if cult ~= self:getfaction() then
                    return false
                end
            end
            return true
        end
        return false
    end

    function artefact:checkfame()
        if self:checkRestriction() then
            local fame = getDbProp(self.fame)
            if self.faction == self:getfaction(fame) then
                return true
            end
        end
        return false
    end

    artefact.__index = artefact
end


function artefact:__init__()
    local vars = {
        bag = "SERVER:INVENTORY:BAG",
        kami = "SERVER:FAME:PLAYER4:VALUE",
        kara = "SERVER:FAME:PLAYER5:VALUE",
        fame = "SERVER:FAME:CULT_ALLEGIANCE",
        dapper = "SERVER:INVENTORY:MONEY",
        filter = "UI:SAVE:INV_BAG:FILTER_TP",
        dopact = "UI:SAVE:TELEPORT:DO_PACT",
        banner = "UI:SAVE:TELEPORT:BANNER",
        active = "UI:SAVE:ISACTIVE:ARTEFACT",
        detach = "UI:SAVE:TELEPORT:ISDETACHED",
        closeTp = "UI:SAVE:TELEPORT:CLOSE_AFTER_TP",
        -- minimum fame required
        threshold = 33,
        max_slots = 500,
        loadTpTime = 16,
        pacts = {
            -- ecosystem
            fyros = {
                -- sheetName = price
                tp_f_pyr = 1000,
                tp_f_dyron = 1000,
                tp_f_thesos = 1000,
                tp_f_oflovaksoasis = 1500,
                tp_f_frahartowers = 2500,
                tp_f_sawdustmines = 2500,
                tp_f_outlawcanyon = 4000,
                tp_f_thescorchedcorridor = 6000
            },
            tryker = {
                tp_f_fairhaven = 1000,
                tp_karavan_crystabell = 1000,
                tp_karavan_windermeer = 1000,
                tp_karavan_avendale = 1000,
                tp_f_dewdrops = 1500,
                tp_f_windsofmuse = 1500,
                tp_f_thefount = 2500,
                tp_f_restingwater = 2500,
                tp_f_bountybeaches = 4000,
                tp_f_enchantedisle = 4000,
                tp_f_lagoonsofloria = 6000
            },
            matis = {
                tp_f_yrkanis = 1000,
                tp_karavan_davae = 1000,
                tp_karavan_avalae = 1000,
                tp_karavan_natae = 1000,
                tp_f_fleetinggarden = 1500,
                tp_f_knollofdissent = 2500,
                tp_f_hiddensource = 4000,
                tp_f_hereticshovel = 4000,
                tp_f_upperbog = 4000,
                tp_f_groveofconfusion = 6000
            },
            zorai = {
                tp_f_zora = 1000,
                tp_kami_hoi_cho = 1000,
                tp_kami_jen_lai = 1000,
                tp_kami_min_cho = 1000,
                tp_f_maidengrove = 1500,
                tp_f_havenofpurity = 2500,
                tp_f_groveofumbra = 4000,
                tp_f_knotofdementia = 4000,
                tp_f_thevoid = 6000
            },
            primes = {
                tp_f_ranger_camp = 1000,
                tp_kami_shining_lake = 1000,
                tp_karavan_shattered_ruins = 1000,
                tp_f_almati = 10000,
                tp_f_nexus_terre = 10000,
                tp_f_the_windy_gate = 10000,
                tp_f_the_sunken_city = 10000,
                tp_f_forbidden_depths = 10000,
                tp_f_gate_of_obscurity = 10000,
                tp_f_the_elusive_forest = 10000,
                tp_f_the_land_of_continuity = 10000,
                tp_f_the_under_spring_fyros = 10000,
                tp_f_the_abyss_of_ichor_matis = 10000,
                tp_f_the_trench_of_trials_zorai = 10000
            }
        },
        -- item description to remove
        -- TODO: translation fixes we need unique format
        blacklist = {
            -- kami
            "Pacte kami %/ Téléporteur vers ",                  -- FR
            "Pacte Kami %/ Téléporteur vers ",
            "Соглашение с Ками о перемещении в ",               -- RU
            "Kami Teleportationspakt für Den ",                 -- DE
            "Kami Teleportationspakt für Die ",
            "Kami Teleportationspakt für Das ",
            "Kami Teleportationspakt für das ",
            "Kami Teleportationspakt für den ",
            "Kami Teleportationspakt für ",
            "Kami%-Teleportationspakt für den ",
            "Kami%-Teleportationspakt für das ",
            "Kami%-Teleportationspakt für die ",
            "Kami%-Teleportationspakt für ",
            "Kami Teleporter Pact for the ",                    -- EN
            "Kami Teleporter Pact for ",
            "Pacto Teletransportador Kami para ",               -- ES
            "Pacto Teletransportador para ",
            "Pacto Teletransportador Kamik ",
            "Pacto Kami %/ Teleportador hacia ",
            "Pacto Teletransportador ",
            "Pacto de Teletransportacoin Kama para madera ",
            -- kara
            "Pacte karavan %/ Téléporteur vers ",               -- FR
            "Pacte Karavan %/ Téléporteur vers ",
            "Соглашение с Караваном о перемещении в",           -- RU
            "Karavan Teleportationspakt für Den ",              -- DE
            "Karavan Teleportationspakt für Die ",
            "Karavan Teleportationspakt für Das ",
            "Karavan Teleportationspakt für das ",
            "Karavan Teleportationspakt für ",
            "Karavan%-Teleportationspakt für den ",
            "Karavan%-Teleportationspakt für das ",
            "Karavan%-Teleportationspakt für die ",
            "Karavan%-Teleportationspakt für ",
            "Karavan Teleporter Pact for the ",                 -- EN
            "Karavan Teleporter Pact for ",
            "Pacto Teletransportador Karavan para ",            -- ES
            "Pacto Teletransportdor Karavan para ",
            "Pacto Teletransportador Karavn para ",
            "Pacto Teletransportador para ",
            "Pacto Karavan %/ Teleportador hacia ",
            "Pacto de Teletransportacion Karavan para madera ",
            "Pacto Teletransportador "
        },
        psort = function(p0, p1) return p0.label < p1.label end
    }
    vars.__index = vars

    setmetatable(artefact, vars)
end

function artefact:startInterface(cult)
    if not self.active then
        self:__init__()
    end
    -- kami kara
    if not self:checkRestriction(cult) or not cult then
        displaySystemInfo(i18n.get("uiArtefactRestrict"), "BC")
        self:onClose()
        return
    end
    if not self:checkfame() then
        self.uiWindow = nil
    end
    self.faction = self:getfaction()

    if getDbProp(self.kara) >= self.threshold or
        getDbProp(self.kami) >= self.threshold then
        local tmp = {}
        -- load the bunch of pacts at first run
        if not self.uiWindow then
            for eco, tp in pairs(self.pacts) do
                tmp[eco] = {}
                for sheet, price in pairs(tp) do
                    sheet = string.gsub(sheet, "_f_", "_"..self.faction.."_")
                    -- faction only
                    if self:strcmp(sheet, "tp_"..self.faction) then
                        local sheet_name = string.gsub(sheet, "tp_", "")
                        sheet_name = string.gsub(sheet_name, ".sitem", "")
                        label = self.items_labels[sheet_name]
                        tmp[eco][#tmp[eco]+1] = {
                            name = sheet,
                            desc = getSheetLocalizedName(sheet..".sitem"),
                            label = label,
                            cost = tostring(price)
                        }
                    end
                end
                -- alphabetic order
                table.sort(tmp[eco], self.psort)
            end
            -- update
            self.currentPacts = tmp
        end
        -- window is loaded from xml
        if not self.uiWindow then
            -- reuse the previous frame if exist
            self.uiWindow = getUI(self.idWindow, false)
            if not self.uiWindow then
                return
            end
            -- dimension
            self.uiWindow.w = self.w
            self.uiWindow.h = self.h
            self.uiWindow.pop_max_w = self.max_w
            self.uiWindow.pop_max_h = self.max_h
            self.uiWindow.pop_min_w = self.min_w
            self.uiWindow.pop_min_h = self.min_h
        end
        self:hidePact()
        -- is window attached?
        if getDbProp(self.detach) == 0 then
            if self.observer then
                self:bagObserver(false)
            end
            runAH(getUICaller(), "proc", "artefact_win_attach")
        else
            -- trigger on_open event
            if not self.uiWindow.opened then
                self.uiWindow.opened = true
            end
            -- trigger on_active event
            if not self.uiWindow.active then
                self.uiWindow.active = true
            end
            local html = self.uiWindow:find("html")
            -- render content
            if html then
                self:dynRender(html)
            end
            setTopWindow(self.uiWindow)
            -- auto update
            if not self.observer then
                runAH(getUICaller(), "proc", "artefact_proc_active")
            end
        end
    end
end
--
--

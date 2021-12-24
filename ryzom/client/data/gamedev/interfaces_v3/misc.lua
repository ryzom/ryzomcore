-- misc
--
if misc == nil then
    misc = {}
end

-- convert CRGBA to hexadecimal 6-digits
function misc:toHexRGBA(rgba)
    return string.format("%.2x%.2x%.2x%.2x", rgba.R, rgba.G, rgba.B, rgba.A)
end

-- convert hexadecimal to its short-hand version
function misc:toHexShort(hex)
    local s = ''
    for i = 1, #str, 2 do s = s..string.sub(hex:lower(), i, i) end
    return s
end

-- ready to use colored chat string
function misc:encodeColorTag(hex, str)
    return "@{"..self:toHexShort(hex).."}"..str
end

--
-- inventories observer
function misc:initInvObserver(
    inventory,
    func_add,
    func_del,
    func_change,
    item_filter
)
    if self.invObserver == nil then
        self.invObserver = {
            maxSlots = 499, obs = {}
        }
        function self.invObserver:__remove(n)
            if #self.obs > 0 then
                self.obs[n] = nil
            end
        end
        function self.invObserver:__update(n)
            self.obs[n].cache = nil
            self:__observ(n)
        end
        function self.invObserver:__observ(n)
            local t = {}
            local n = n or 1
            if #self.obs <= 0 then
                return
            end
            for i = 0, self.maxSlots do
                local sheet = getDbProp(self.obs[n].inv..":"..i..":SHEET")
                if sheet > 0 then
                    t[sheet] = getDbProp(self.obs[n].inv..":"..i..":QUANTITY")
                    if self.obs[n].filter then
                        local name = getSheetName(sheet)
                        if not name:match(self.obs[n].filter) then
                            t[sheet] = nil
                        end
                    end
                end
            end
            if not self.obs[n].cache then
                self.obs[n].cache = t
                t = {}
                return
            end
            local lenT = function(t)
                local i = 0
                for _ in pairs(t) do i = i + 1 end return i
            end
            local t_len, b_len = lenT(t), lenT(self.obs[n].cache)
            if t_len ~= b_len then
                if t_len > b_len then
                    self.obs[n].onAdd(n)
                else
                    self.obs[n].onDel(n)
                end
                self:__update(n)
                return
            end
            for k, v in pairs(self.obs[n].cache) do
                local found = false
                for k1, v1 in pairs(t) do
                    if k == k1 then found = true
                        break
                    end
                end
                if found then
                    for k1, v1 in pairs(t) do
                        if k == k1 then
                            if v == v1 then break end
                            self.obs[n].onChange(n)
                            self:__update(n)
                        end
                    end
                else
                    self:__update(n)
                end
            end
        end
        function self.invObserver:add(uiWindow, n)
            if uiWindow then
                setOnDraw(uiWindow, formatUI("misc.invObserver:__observ(#1)", n))
            end
        end
        function self.invObserver:remove(uiWindow, n)
            if uiWindow then
                setOnDraw(uiWindow, "")
                self:__remove(n)
            end
        end
        self.invObserver.__index = self.invObserver
    end
    if inventory then
        self.invObserver.obs[#self.invObserver.obs+1] =
        {
            inv = inventory,
            onAdd = func_add,
            onDel = func_del,
            onChange = func_change,
            filter = item_filter,
            cache = nil
        }
    end
    return setmetatable(misc, self.invObserver)
end
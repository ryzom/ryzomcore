
if not WebQueue then
	WebQueue = {doinit=true, web="", url="", queue={}, updates={}, tries=0}
	-- TODO: replace with runAH(nil, "add_link", ...)
end

function WebQueue:debug(text)
	debug(text)
end

function WebQueue:push(url, web)
	for k,v in pairs(self.queue) do
		if v[1] == url and v[2] == web then
			self:debug("Url allready in queue : "..url)
			return
		end
	end

	table.insert(self.queue, {url, web, 2})
end

function WebQueue:update(url, web, timer)
	for k,v in pairs(self.updates) do
		if v[1] == url and v[2] == web then
			self:debug("Update already added")
			if timer == 0 then
				self:debug("Remove update...")
				table.remove(self.updates, k)
			else
				self.updates[k][3]=timer
			end
			return
		end
	end

	table.insert(self.updates, {url, web, timer, 0})
end

function WebQueue:load(url, web)
	self.url = url
	self.web = "ui:interface:" .. (web or "webqueue")
	self.loaded = false
	self.tries = 3
	self:reload()
	self:debug("++ loading url [" .. self.url .."] into [" .. self.web .. "]")
end

function WebQueue:reload()
	local html = getUI(self.web):find("html")
	if html == nil then
		self:debug(self.web.." not found...")
	else
		html:removeContent()
		html:browse(self.url)
	end
	self.tries = self.tries - 1
end

function WebQueue:loop()
	if self.url ~= "" and self.tries < 0 then
		self:debug("Giving up loading url [" .. self.url .. "]")
		self.url = ""
	end

	if self.url == "" then
		-- Check updates
		for k,v in pairs(self.updates) do
			v[4] = v[4] - 1
			if v[4] < 0 then
				v[4] = v[3] -- reset timer
				WebQueue:push(v[1], v[2])
			end
		end

		if self.queue[1] then
			-- delay request few ticks so that url pushed from browser <lua> tags
			-- will not interfere browsing
			self.queue[1][3] = self.queue[1][3] - 1
			self:debug("Next request in: " .. tostring(self.queue[1][3]))
			if self.queue[1][3] <= 0 then
				self:load(self.queue[1][1], self.queue[1][2])
				table.remove(self.queue, 1)
			end
		end
		return
	end

	local elm = getUI(self.web)
	if not elm then
		self:debug(self.web.." not found...")
		return
	end
	elm = elm:find("html:text_list")
	if not elm then
		self:debug(self.web..":html:text_list not found...")
		return
	end

	elm = elm:getChild(0):find("")
	if not elm then
		return
	end
	-- [Error : Connection timeout : ]
	-- [Error : The page cannot be displayed : ]
	if elm.hardtext then
		if string.sub(elm.hardtext, 0, 8) == "Error : " then
			self:debug("loading failed, retrying in 3..2..1..")
			self:load(self.url)
		else
			if getUI(self.web).title ~= nil then
				if string.sub(getUI(self.web).title, 0, 11) ~= "Please wait" then
					self:debug("loading done")
					self.url = ""
				end
			else
				--self:debug("check loaded")
				--if self.loaded == true then
					self:debug("loading done")
					self.url = ""
				--end
			end
		end
	else
		self.url = ""
	end
end

if WebQueue.doinit then
	WebQueue.doinit = false
	addOnDbChange(getUI("ui:interface:webqueue"), "@UI:VARIABLES:CURRENT_SERVER_TICK", "WebQueue:loop()")
end


-- VERSION --
RYZOM_WEB_QUEUE_VERSION = 324

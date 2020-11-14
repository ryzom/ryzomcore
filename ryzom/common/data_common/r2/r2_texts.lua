local registerFeature = function()
	local feature = {}
	feature.Name = "TextManager"

	feature.Description = "A little texts manager"
	feature.Components=
	{
		{
			Name="TextManager",
			Prop=
				{
					{Name="InstanceId",Type="String"},
					{Name="Texts",Type="Table"},
				}
		},
		{
			Name="TextManagerEntry",
			Prop=
				{	
					{Name="InstanceId", Type="String" },
					{Name="Text",Type="String"},
					{Name="Count",Type="Number"}
				}
		}

	}


	--returns nil if the text is still in the textManager
	--else, return a new entry, not inserted in the TextManager
	feature.checkText = function(textManager,text)
		local texts = r2.Scenario.Texts.Texts
		k,entry = next(texts,nil)
		while k ~=nil do
			local textEntry = entry.Text
			if textEntry==text
			then
				return entry
			end
			k,entry = next(texts,k)
		end
		entry = r2.newComponent("TextManagerEntry")
		entry.Text=text
		entry.Count=0
		return entry
	end

	feature.getText = function (textManager, entry)

		debugInfo("feature.getText")

		for i=0, textManager.Texts.Size-1 do

			local text = textManager.Texts[i]

			if text.InstanceId == entry
			then
				return text.Text
			end
		end
		return nil
	end

	--add a text to the text manager.
	--if the text exist, increment its counter
	feature.addText = function(textManager,text)
		local max = table.getn(textManager.Texts)
		local entry 
		
		for i=1,max do
			entry = textManager.Texts[i]
			if entry.Text==text
			then
				entry.Count = entry.Count+1 
				return entry
			end
		end

		entry = r2.newComponent("TextManagerEntry")
		entry.Text=text
		entry.Count=1
		table.insert(textManager.Texts,entry)
		return entry
	end

	--decrement the counter of a text.
	--if this counter==0, remove the text from the TextManager	
	feature.removeText = function(textManager, text)

		for i=0,textManager.Texts.Size-1  do
			local entry = textManager.Texts[i]
			if entry.Text==text then

				local newCount = entry.Count-1
				
				if newCount==0 then
					r2.requestEraseNode(entry.InstanceId, "", -1)
				else
					r2.requestSetNode(entry.InstanceId, "Count", newCount)
				end
			end
		end
	end	

	

	feature.Translator = function(context)
		local comp = context.Feature
		local texts = context.Feature.Texts
		local entry
		local rtTextMgr = context.RtScenario.Texts
		--for each entry
		local k,v = next(texts,nil)
		while v~=nil do
			if k~="Keys"
			then
				--create and fill a RT entry
				entry = r2.newComponent("RtEntryText")
				entry.Text = v.Text
				--insert it in the RT text manager
				table.insert(rtTextMgr.Texts,entry)
				local tmp = {}
				table.insert(tmp,v.InstanceId)
				table.insert(tmp,entry.Id)
				table.insert(context.TextTranslateId,tmp)
			end
			k,v = next(texts,k)
		end
		if table.getn(context.TextTranslateId)==0
		then
			debugInfo("translator:: pas d'entrees dans la table!!!")
		end
	end

	-- ?
	feature.getRtId = function(context, instanceId)
		local tab = context.TextTranslateId
		assert(tab~=nil)
		for k,v in pairs(tab)
		do	
			if instanceId == v[1]
			then
				return v[2]
			end
		end
		local max = table.getn(tab)
		if max==0
		then
			debugInfo(colorTag(255, 255, 0).."WAR: The text table is empty.")
			return
		end

		for i=1,max do
			if instanceId == tab[i][1]
			then			
				return tab[i][2]
			end
		end
	end

	return feature
end

r2.Features["TextManager"] = registerFeature()

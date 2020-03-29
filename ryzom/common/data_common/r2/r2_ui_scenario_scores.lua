-------------------------------------------------------------------------------------------------------------
----------------------------------   SCENARIO SCORES       --------------------------------------------------
-------------------------------------------------------------------------------------------------------------

ScenarioScores = 
{
	emptyScore = "",
	emptyAverage = "-",
}

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:getWindow()
	local ui = getUI("ui:interface:r2ed_scenario_scores")
	assert(ui)
	return ui
end

--------------------------------------------------------------------------------------------------------------
--

function ScenarioScores:initScenarioScores()

end

function ScenarioScores:initScenarioScoresOLD()

	local ui = self:getWindow()

	local noRatingsGr = ui:find("no_ratings")
	assert(noRatingsGr)

	local ratingsGr = ui:find("ratings")
	assert(ratingsGr)

	local averagesGr = ui:find("average_ratings")
	assert(averagesGr)

	noRatingsGr.active= not isInRingMode()
	if isInRingMode() then

		local isDm = false
		if r2.Mode == "DM" or r2.Mode == "AnimationModeDm" then isDm = true end

		local header = r2.getScenarioHeader()
		local seeAverages = (isDm or r2.hasCharacterSameCharacterIdMd5(tostring(header.CreatorMD5)))
		ratingsGr.active = not seeAverages
		averagesGr.active = seeAverages
	else
		ratingsGr.active = false
		averagesGr.active = false	
	end

	if isInRingMode() then

		if ratingsGr.active then
			if game.getScenarioScores then	
				game.getScenarioScores()
			end

			local rateFunUI =			ratingsGr:find("fun_rate"):find("edit_rate"):find("eb")
			rateFunUI.input_string=	self.emptyScore
			
			local rateDifficultyUI =	ratingsGr:find("diff_rate"):find("edit_rate"):find("eb")
			rateDifficultyUI.input_string= self.emptyScore
			
			local rateAccessibilityUI = ratingsGr:find("access_rate"):find("edit_rate"):find("eb")
			rateAccessibilityUI.input_string= self.emptyScore
			
			local rateOriginalityUI =	ratingsGr:find("originality_rate"):find("edit_rate"):find("eb")
			rateOriginalityUI.input_string=	self.emptyScore
			
			local rateDirectionUI =		ratingsGr:find("direction_rate"):find("edit_rate"):find("eb")
			rateDirectionUI.input_string= self.emptyScore

		else
			if game.getSessionAverageScores then	
				game.getSessionAverageScores()
			end

			local rateFunUI =			averagesGr:find("fun_rate"):find("score_text")
			rateFunUI.hardtext=			self.emptyAverage
			
			local rateDifficultyUI =	averagesGr:find("diff_rate"):find("score_text")
			rateDifficultyUI.hardtext=	self.emptyAverage
			
			local rateAccessibilityUI = averagesGr:find("access_rate"):find("score_text")
			rateAccessibilityUI.hardtext= self.emptyAverage
			
			local rateOriginalityUI =	averagesGr:find("originality_rate"):find("score_text")
			rateOriginalityUI.hardtext=	self.emptyAverage
			
			local rateDirectionUI =		averagesGr:find("direction_rate"):find("score_text")
			rateDirectionUI.hardtext=	self.emptyAverage
		end
	end

	-- windows title
	if ratingsGr.active then
		ui.uc_title = i18n.get("uiR2EDMyScenarioScores")
	else
		ui.uc_title = i18n.get("uiR2EDScenarioScores")
	end
end

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:onScenarioScoresReceived(scores)

	self:fill(scores)
end

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:onAverageScoresReceived(averages)

	self:fillAverages(averages)
end

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:onScenarioAverageScoresReceived(averages)

	self:fillScenarioAverages(averages)
end

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:fill(scores)

	local ui = self:getWindow()
	local ratingsGr = ui:find("ratings")
	assert(ratingsGr)

	local rateFunUI =			ratingsGr:find("fun_rate"):find("edit_rate"):find("eb")
	if scores.ScenarioRated==0 then 
		rateFunUI.input_string=			tostring(self.emptyScore)
	else
		rateFunUI.input_string=			tostring(scores.RateFun)
	end
	
	local rateDifficultyUI =	ratingsGr:find("diff_rate"):find("edit_rate"):find("eb")
	if scores.ScenarioRated==0 then 
		rateDifficultyUI.input_string=			tostring(self.emptyScore)
	else
		rateDifficultyUI.input_string=			tostring(scores.RateDifficulty)
	end

	local rateAccessibilityUI = ratingsGr:find("access_rate"):find("edit_rate"):find("eb")
	if scores.ScenarioRated==0 then 
		rateAccessibilityUI.input_string=			tostring(self.emptyScore)
	else
		rateAccessibilityUI.input_string=			tostring(scores.RateAccessibility)
	end

	local rateOriginalityUI =	ratingsGr:find("originality_rate"):find("edit_rate"):find("eb")
	if scores.ScenarioRated==0 then 
		rateOriginalityUI.input_string=			tostring(self.emptyScore)
	else
		rateOriginalityUI.input_string=			tostring(scores.RateOriginality)
	end

	local rateDirectionUI =		ratingsGr:find("direction_rate"):find("edit_rate"):find("eb")
	if scores.ScenarioRated==0 then 
		rateDirectionUI.input_string=			tostring(self.emptyScore)
	else
		rateDirectionUI.input_string=			tostring(scores.RateDirection)
	end
end

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:fillAverages(averages)

	local ui = self:getWindow()
	local averagesGr = ui:find("average_ratings")
	assert(averagesGr)

	local rateFunUI =			averagesGr:find("fun_rate"):find("score_text")
	if averages.ScenarioRated==0 then 
		rateFunUI.hardtext=		self.emptyAverage
	else
		rateFunUI.hardtext=			tostring(math.min(100, averages.RateFun))
	end
	
	local rateDifficultyUI =	averagesGr:find("diff_rate"):find("score_text")
	if averages.ScenarioRated==0 then 
		rateDifficultyUI.hardtext=		self.emptyAverage
	else
		rateDifficultyUI.hardtext=	tostring(math.min(100, averages.RateDifficulty))
	end
	
	local rateAccessibilityUI = averagesGr:find("access_rate"):find("score_text")
	if averages.ScenarioRated==0 then 
		rateAccessibilityUI.hardtext=		self.emptyAverage
	else
		rateAccessibilityUI.hardtext=	tostring(math.min(100, averages.RateAccessibility))
	end
	
	local rateOriginalityUI =	averagesGr:find("originality_rate"):find("score_text")
	if averages.ScenarioRated==0 then 
		rateOriginalityUI.hardtext=		self.emptyAverage
	else
		rateOriginalityUI.hardtext=	tostring(math.min(100, averages.RateOriginality))
	end
	
	local rateDirectionUI =		averagesGr:find("direction_rate"):find("score_text")
	if averages.ScenarioRated==0 then 
		rateDirectionUI.hardtext=		self.emptyAverage
	else
		rateDirectionUI.hardtext=	tostring(math.min(100, averages.RateDirection))
	end
end

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:fillScenarioAverages(averages)

	local ui = getUI("ui:interface:ring_scenario_loading_window")--self:getWindow()
	assert(ui)
	
	local rateFunUI =			ui:find("FunRating")
	rateFunUI.hardtext=			tostring(math.min(100, averages.RateFun))
	
	local rateDifficultyUI =	ui:find("DifficultyRating")
	rateDifficultyUI.hardtext=	tostring(math.min(100, averages.RateDifficulty))
	
	local rateAccessibilityUI = ui:find("AccessibilityRating")
	rateAccessibilityUI.hardtext=	tostring(math.min(100, averages.RateAccessibility))
	
	local rateOriginalityUI =	ui:find("OriginalityRating")
	rateOriginalityUI.hardtext=	tostring(math.min(100, averages.RateOriginality))
	
	local rateDirectionUI =		ui:find("DirectionRating")
	rateDirectionUI.hardtext=	tostring(math.min(100, averages.RateDirection))

	local rrpTotal =		ui:find("RRPTotal")
	rrpTotal.hardtext=	tostring(math.min(100, averages.RRPTotal))

	local rollouts = ui:find("rollouts")
	local deltaH = 40								
	ui:invalidateCoords()
	ui:updateCoords()
	local newHReal = rollouts.h_real				
	-- must resize the parent
	local newH = newHReal + deltaH
	local yOffset = newH - ui.h
	--propertySheet.h = newH
	ui.y = ui.y + yOffset / 2 
	ui.pop_min_h = newH 
	ui.pop_max_h = newH 
	ui:invalidateCoords()
	ui:updateCoords()

end

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:limitRating()

	local editBox = getUICaller()
	local rating = editBox.input_string
	
	if rating~="" and tonumber(rating) > 100 then
		editBox.input_string = tostring(100)
	end
end

--------------------------------------------------------------------------------------------------------------
--
function ScenarioScores:updateScores()

	local ui = self:getWindow()
	
	local rateFunUI = ui:find("fun_rate"):find("edit_rate"):find("eb")
	local rateFun = 0
	if rateFunUI.input_string~="" and rateFunUI.input_string~=self.emptyScore then
		rateFun = math.min(100, tonumber(rateFunUI.input_string))
	end

	local rateDifficultyUI = ui:find("diff_rate"):find("edit_rate"):find("eb")
	local rateDifficulty = 0
	if rateDifficultyUI.input_string~=self.emptyScore then
		rateDifficulty = math.min(100, tonumber(rateDifficultyUI.input_string))
	end

	local rateAccessibilityUI = ui:find("access_rate"):find("edit_rate"):find("eb")
	local rateAccessibility = 0
	if rateAccessibilityUI.input_string~=self.emptyScore then
		rateAccessibility = math.min(100, tonumber(rateAccessibilityUI.input_string))
	end

	local rateOriginalityUI = ui:find("originality_rate"):find("edit_rate"):find("eb")
	local rateOriginality = 0
	if rateOriginalityUI.input_string~=self.emptyScore then
		rateOriginality = math.min(100, tonumber(rateOriginalityUI.input_string))
	end

	local rateDirectionUI =	ui:find("direction_rate"):find("edit_rate"):find("eb")
	local rateDirection = 0
	if rateDirectionUI.input_string~=self.emptyScore then
		rateDirection = math.min(100, tonumber(rateDirectionUI.input_string))
	end

	if game.updateScenarioScores then	
		game.updateScenarioScores(rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection)
	end

	ui.active=false
end


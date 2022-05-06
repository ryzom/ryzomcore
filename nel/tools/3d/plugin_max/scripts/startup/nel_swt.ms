plugin SimpleMod SWT
name:"SWTPlug"
classID:#(0x9765367, 0x4b4a356a)
version:1
(

	parameters main rollout:params
	(
		swtPosValue type:#float ui:spinPosRatio default:0.0 animatable:false
		swtRotValue type:#float ui:spinRotRatio default:0.0 animatable:false
		swtScaleValue type:#float ui:spinScaleRatio default:0.0 animatable:false
	)
	-- 
	rollout params "Skeleton Weight Template"
	(
		group "Global"
		(
			spinner spinGlobalRatio "Global Ratio: " type:#float range:[0.0,1.0,0.0] scale:0.01
		)
		group "Details"
		(
			spinner spinPosRatio "Pos Ratio: " type:#float range:[0.0,1.0,0.0] scale:0.01
			spinner spinRotRatio "Rot Ratio: " type:#float range:[0.0,1.0,0.0] scale:0.01
			spinner spinScaleRatio "Scale Ratio: " type:#float range:[0.0,1.0,0.0] scale:0.01
		)
		-- When the global ratio is changed the value is propagated to all spinners
		on spinGlobalRatio changed newValue do
		(
			spinPosRatio.value = newValue
			spinRotRatio.value = newValue
			spinScaleRatio.value = newValue		
		)
	)
)
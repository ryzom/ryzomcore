if (game==nil) then
	game= {};
end

game.RawCreaturesMats = {
  amber = { "m0015dxa", "m0050dxa", "m0124dxa", "m0117dxa", "m0155dxa", "m0102dxa" },
  wood = { "m0128dxa", "m0064dxa", "m0093dxa", "m0001dxa", "m0040dxa" },
  moss = { "m0147chl", "m0525cpe", "m0575cpa", "m0640cpc", "m0658cpf", "m0660cpd", "m0661cpb" },
  bark = { "m0101dxa", "m0630dxa", "m0014dxa", "m0623dxa", "m0497dxa" },
  trunk = { "m0087chh", "m0547chq", "m0678chu" },
  wing = { "m0335ccj", "m0366cbc", "m0564cba", "m0570cbb", "m0596ckj", "m0609cki" },
  rostrum = { "m0074cke" },
  fiber = { "m0118dxa", "m0021dxa", "m0037dxa", "m0006dxa" },
  resin = { "m0534dxa", "m0624dxa", "m0046dxa", "m0541dxa" },
  shell = { "m0053dxa", "m0125dxa", "m0123dxa", "m0031dxa", "m0016dxa" },
  pelvis = { "m0500chw", "m0504cha", "m0507chn", "m0510chb", "m0515chr", "m0546chq", "m0555chx", "m0612chg", "m0616chv", "m0620chh", "m0638cht", "m0645chm", "m0650chp", "m0656chs", "m0665chk", "m0676chu", "m0683chl" },
  sap = { "m0535dxa", "m0142dxa", "m0109dxa", "m0119dxa", "m0533dxa" },
  leather = { "m0044cca", "m0107cce", "m0137ccd", "m0141cch", "m0145cbc", "m0369ccf", "m0371cbb", "m0372cba", "m0376cck", "m0463ccg", "m0574ccj", "m0579ccm", "m0670cco", "m0687ccp", "m0694cci" },
  node = { "m0679dxa", "m0100dxa", "m0662dxa", "m0629dxa", "m0652dxa" },
  aranawood = { "m0469chw" },
  horn = { "m0018chb", "m0136ccd" },
  hoof = { "m0025chc", "m0378chf" },
  skull = { "m0284cda", "m0285cdb", "m0286cdc" },
  fang = { "m0135ccd", "m0140cch", "m0346cce", "m0347cca", "m0348cck", "m0349ccp", "m0356cci", "m0385ccm", "m0465ccg", "m0626ccf", "m0632ccl", "m0668cco" },
  kitin = { "m0522ccb", "m0528che", "m0549ccc", "m0557ccn", "m0581ckg", "m0584cka", "m0586ckd", "m0588ckb", "m0591ckh", "m0599cke", "m0601ckf" },
  mandible = { "m0481ckb", "m0487cka", "m0521ccb", "m0527che", "m0548ccc", "m0556ccn", "m0580ckg", "m0585ckd", "m0590ckh", "m0593ckj", "m0598cke", "m0600ckf", "m0606cki" },
  tail = { "m0078cki", "m0488cka", "m0524ccb", "m0529che", "m0551ccc", "m0559ccn", "m0583ckg", "m0587ckd", "m0589ckb", "m0592ckh", "m0595ckj", "m0603ckf" },
  spine = { "m0009cha", "m0082chg", "m0677chu" },
  claw = { "m0043cca", "m0106cce", "m0134ccd", "m0154cci", "m0386cco", "m0387cka", "m0390ccg", "m0467ccl", "m0468ccf", "m0526che", "m0530cck", "m0577ccm", "m0597cke", "m0671cch", "m0685ccp" },
  ligament = { "m0531cck", "m0542cca", "m0562cba", "m0568cbb", "m0573ccj", "m0578ccm", "m0627ccf", "m0633ccl", "m0641cce", "m0666ccd", "m0669cco", "m0672cch", "m0673ccg", "m0681cbc", "m0686ccp", "m0693cci" },
  seed = { "m0023dxa", "m0659dxa", "m0115dxa", "m0113dxa" },
  tooth = { "m0345chk", "m0350chf", "m0383chx", "m0501chw", "m0505cha", "m0508chn", "m0511chb", "m0516chr", "m0520chc", "m0617chv", "m0639cht", "m0646chm", "m0651chp", "m0657chs" },
  whiskers = { "m0083chg" },
  oil = { "m0103dxa", "m0610dxa", "m0049dxa", "m0565dxa" },
  beak = { "m0560cba", "m0566cbb", "m0571ccj", "m0680cbc" },
  hairs = { "m0295cdb", "m0307cda" },
  bud = { "m0472cpa", "m0473cpb", "m0474cpc", "m0475cpd", "m0476cpe", "m0477cpf" },
  bone = { "m0153cci", "m0338cch", "m0339cca", "m0341cck", "m0343cce", "m0384ccl", "m0462cbc", "m0464ccg", "m0561cba", "m0567cbb", "m0572ccj", "m0576ccm", "m0625ccf", "m0667cco", "m0684ccp" },
  nail = { "m0020chb", "m0149chl", "m0359chf", "m0374chn", "m0499chw", "m0514chr", "m0519chc", "m0545chq", "m0554chx", "m0615chv", "m0619chh", "m0637cht", "m0644chm", "m0649chp", "m0655chs", "m0664chk" },
  skin = { "m0019chb", "m0081chg", "m0086chh", "m0133chk", "m0363chu", "m0364chr", "m0365chf", "m0367chn", "m0471chv", "m0503cha", "m0518chc", "m0544chq", "m0553chx", "m0636cht", "m0643chm", "m0648chp", "m0654chs", "m0682chl" },
  eyes = { "m0498chw", "m0502cha", "m0506chn", "m0509chb", "m0512chr", "m0517chc", "m0543chq", "m0552chx", "m0611chg", "m0613chv", "m0618chh", "m0621chf", "m0635cht", "m0642chm", "m0647chp", "m0653chs", "m0663chk", "m0675chu" },
  sting = { "m0067ckd", "m0076ckf", "m0480ckb", "m0496ckh", "m0523ccb", "m0550ccc", "m0558ccn", "m0582ckg", "m0594ckj", "m0608cki" },
  kitinshell = { "m0048ccc", "m0066ckd", "m0068ckg", "m0069ckj", "m0072ckh", "m0073cke", "m0336ccb", "m0368ccn", "m0470che", "m0479ckb", "m0485cka", "m0602ckf", "m0607cki", "m0634ccl" },
  mushroom = { "m0148chl" }
}

-- VERSION --
RYZOM_RAW_CREATURE_MATS_VERSION = 366
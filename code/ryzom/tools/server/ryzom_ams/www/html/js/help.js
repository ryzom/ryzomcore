function show_help(help_tip)
{
	if(help_tip =='test')
	{
		var tour = new Tour();
		tour.addStep({
			element: ".brand:first", /* html element next to which the step popover should be shown */
			placement: "bottom",
			title: "Custom Tour", /* title of the popover */
			content: "You can create tour like this. Click Next." /* content of the popover */
		});
		tour.addStep({
			element: ".theme-container",
			placement: "left",
			title: "Themes",
			content: "You change your theme from here."
		});
		tour.addStep({
			element: "ul.main-menu a:first",
			title: "Dashboard",
			content: "This is your dashboard from here you will find highlights."
		});
		tour.restart();
	}

}



function comboInit(thelist)
{
  theinput = document.getElementById(theinput);  
  var idx = thelist.selectedIndex;
  var content = thelist.options[idx].value;
  if(theinput.value == "")
    theinput.value = content;	
}

function combo(thelist, theinput)
{
  theinput = document.getElementById(theinput);  
  var idx = thelist.selectedIndex;
  var content = thelist.options[idx].value;
  theinput.value = content;	
}

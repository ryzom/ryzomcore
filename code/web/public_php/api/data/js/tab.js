
if(window.addEventListener)
    window.addEventListener("load", tabulation, false);
else
    window.attachEvent("onload", tabulation);
    
function tabulation(){
    var textareas = document.getElementsByTagName("textarea");
    for(var i = 0, t = textareas.length; i < t; i++){
        textareas[i].onkeydown = function(e){
            var tab = (e || window.event).keyCode == 9;
            if(tab){
                var tabString = String.fromCharCode(9);
                var scroll = this.scrollTop;
                
                if(window.ActiveXObject){
                    var textR = document.selection.createRange();
                    var selection = textR.text;
                    textR.text = tabString + selection;
                    textR.moveStart("character",-selection.length);
            textR.moveEnd("character", 0);
                    textR.select();
                }
                else {
                    var beforeSelection = this.value.substring(0, this.selectionStart);
                    var selection = this.value.substring(this.selectionStart, this.selectionEnd);
                    var afterSelection = this.value.substring(this.selectionEnd);
                    this.value = beforeSelection + tabString + selection + afterSelection;
                    this.setSelectionRange(beforeSelection.length + tabString.length, beforeSelection.length + tabString.length + selection.length);
                }                
                this.focus();
                this.scrollTop = scroll;
                return false;
            }
        };        
    }
}
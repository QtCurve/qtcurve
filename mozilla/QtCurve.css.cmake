@namespace url("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul");

/*
  This file modifies Firefox (and other Mozilla apps?) so that they behave in a more
  KDE like manner
*/

/* Swap Cancel/OK -> OK/Cancel */
.dialog-button-box { -moz-box-direction: reverse; -moz-box-pack: right; }
.dialog-button-box spacer { display: none !important; }

/* FireFox >=1.5.x */
prefwindow { -moz-binding: url("file://@GTK_THEME_DIR@/mozilla/preferences-rev.xml#prefwindow") !important; }

/* Dont embolden the text on the selected tab */
tab[selected="true"] { font-weight: normal !important; }

/* Buttons */
/* The following are commented-out, becuase they cause errors with FireFox 2.x :-( To re-eanble, 
   delete all lines with COMMENT_OUT on */

/* COMMENT_OUT
button label,
button image,
toolbarbutton label,
toolbarbutton image,
button[disabled="true"]:hover:active label,
button[disabled="true"]:hover:active image,
toolbarbutton[disabled="true"]:hover:active label,
toolbarbutton[disabled="true"]:hover:active image
{
  padding-left: 0px;
  padding-top: 0px;
  padding-right: 1px;
  padding-bottom: 1px;
}

button:hover:active label,
button:hover:active image,
toolbarbutton:hover:active label,
toolbarbutton:hover:active image
{
  padding-left: 1px;
  padding-top: 1px;
  padding-right: 0px;
  padding-bottom: 0px;
}

COMMENT_OUT */

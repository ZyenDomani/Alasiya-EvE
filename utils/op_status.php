<tr><td class="content">
<h1><center><span class="mw-headline" id="Crucible">Alasiya's Crucible Server</span></center></h1>
<table style="background: transparent; width:100%"><tr>
<td style="text-align: center;">Estimated Overall Progress&#160;&#160;</td></tr></table><div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:64%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table></td></tr></table>
<table style="background: transparent; width:100%"><tr><td style="text-align: center;">
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:64%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>63.9</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;"><BR>
Items with date style DayMonthYear as last line in comments are accurate for Alasiya and those working can be tested on main server.<BR>
Please Note: This page is in the process of being updated for Alasiya's Code Status.<BR>
Please Also Note:  Items listed as "NOT IMPLEMENTED" means there is no code written for that particular system, and the server will not respond to requests for that system.<BR>
Items listed as "Skeleton Framework Only" has basic code, with limited or no processing and/or returns, and the server will not respond, or only partially respond to these requests.<BR><BR>
Last Update: <font color="green">1 December 2017</font><BR>
Page Status: <font color="green">Current</font><BR><BR></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="System_Foundation_Fundamentals"> System Foundation Fundamentals </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:93%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:93%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>93.3</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;"><tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td><td style="width: 26%;">Comments</td></tr>
<tr><td> - Stable, Efficient Server Code</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Progress - eliminating duplicated code and headers, tracking/fixing segfaults, updating code to C++/0x11 and to conform to style guidelines from Scott Meyers' Effective C++.<BR>27Feb16</td></tr>
<tr><td> - Efficient, Accurate Packet Deciphering</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Progress - Most packets understood and correctly processed<BR>27July15</td></tr>
<tr><td> - Further Mutex-ing</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Progress - Testing for intermittent data races<BR>10Aug16</td></tr>
<tr><td> - COMPLETE Destiny rewrite</td><td class="working">99%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 99%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Final test phase <BR>27July16</td></tr>
<tr><td> - COMPLETE Module Class rewrite</td><td class="working">99%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 99%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Complete<BR>27Jun17</td></tr>
<tr><td> - COMPLETE SystemEntity rewrite</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - COMPLETE Object Attribute Management overhaul</td><td class="working">99%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 99%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Complete<BR>27Jun17</td></tr>
<td> - COMPLETE investigation of remaining database fields</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Complete<BR>27Jun17</td></tr>
<tr><td> - Proper spawning and item creating(Item class rewrite)</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Final test phase <BR>27April16</td></tr>
<tr><td> - Eliminate all compiler Warnings from the Alasiya EVEmu solution</td><td class="working">99%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 99%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Complete - some warnings cannot be eliminated due to virual class' data and function variables.<BR>27Jun17</td></tr>
<tr><td> - GM commands</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Complete<BR>27Jun17</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="General_Interface_Features"> General Interface Features </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:73%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:73%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>73.5</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Get Item Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - Get Ship Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - Get Skill Info</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Get Certification Info</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Get Character Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - Get NPC Corporation Info</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Progress<BR>7Sep14</td></tr>
<tr><td> - Get Player Corporation Info</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Account_and_Character_Management"> Account and Character Management </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:96%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table></td></tr></table></div></div>
<div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td><table style="background: transparent; width:96%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>96.3</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Account Login</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE.  All Data is Dynamic now.<BR>15Feb14</td></tr>
<tr><td> - Account Banning/Kicking</td><td class="working">60%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 60%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Progress</td></tr>
<tr><td> - Character Creation</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>15Feb14</td></tr>
<tr><td> - Character Deletion</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Working, incomplete<BR>1Feb14</td></tr>
<tr><td> - Character entrance to last location</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - Character Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>17Nov17</td></tr>
<tr><td> - Open Skills Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - Open Certificate Window and Planner</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - Open Decorations Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE - Decorations not Implemented<BR>10Aug16</td></tr>
<tr><td> - Open Attributes Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - Neural Remap</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00">
</td><td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - Open Augmentations Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE - Implants not implemented<BR>1Feb14</td></tr>
<tr><td> - Open Jump Clones Window</td><td class="broken">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE - Jump Clones not implemented<BR>1Feb14</td></tr>
<tr><td> - Open Bio Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - Open Employment History Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>4Apr14</td></tr>
<tr><td> - Open Standings Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE -  Standings paritially Implemented - only factions at this time<BR>27July15</td></tr>
<tr><td> - Open Security Status Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE - SecStatus partially Implemented - changes not saved to db yet.<BR>1Feb14</td></tr>
<tr><td> - Open Kill Rights Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE - Kill Rights not Implemented<BR>1Feb14</td></tr><tr>
<td> - Open Combat Log Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE - Combat Log not Implemented<BR>27July15</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Skills_-_Certificates"> Skills &amp; Certificates </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>&nbsp;</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td><font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Browse Skills to train</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<br>6Mar14</td></tr>
<tr><td> - Add skills to queue</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<br>6Mar14</td></tr>
<tr><td> - Filter skills that don't fit in queue time remaining</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<br>6Mar14</td></tr>
<tr><td> - Apply changes to skill queue</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<br>6Mar14</td></tr>
<tr><td> - Pause skill queue</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<br>6Mar14</td></tr>
<tr><td> - Re-Start skill queue after pause</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - Skill training time</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>6Mar14</td></tr>
<tr><td> - Right-click add skill to queue from Skills Window in Character Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<br>6Mar14</td></tr>
<tr><td> - Certificate Awarding</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<br>27July15</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="NPC_Station_Services"> NPC Station Services </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:83%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:83%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>82.7</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Right-click Menu in Station</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - In-station Guests List</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can repackage ships/items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Can assemble ships and items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>21Feb14</td></tr>
<tr><td> - Can eject from ship to pod</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Nov15</td></tr>
<tr><td> - Can make active ships</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Nov15</td></tr>
<tr><td> - Trade Services</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Complete - corp trade not coded.<BR>13Dec15</td></tr>
<tr><td> - Repair Services</td><td class="broken">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Initial test phase <BR>27April16</td></tr>
<tr><td> - Insurance Services</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>22Aug15</td></tr>
<tr><td> - Bounty Services</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>22Aug15</td></tr>
<tr><td> - Market Services</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Complete - Few Odd Bugs<BR>30Dec14</td></tr>
<tr><td> - Medical Services</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Few Odd Bugs<BR>30Dec14</td></tr>
<tr><td> - LP Services</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr>
<tr><td> - Clone upgrade</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Clone transfer</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only<BR>7Sep14</td></tr>
<tr><td> - Jump clone installation</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr>
<tr><td> - Items window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>21Feb14</td></tr>
<tr><td> - Can merge/stack items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Can trash and split items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Can open/close containers</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - Can move items into/out of containers</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - Can inject skills from items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Agents_Missions"> NPC Agents and Missions </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:42%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:42%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>42.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Agents Tab</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Agents Tab Working - have to click between guests and agents to display agents<BR>7Sep14</td></tr>
<tr><td> - Can initiate conversation with agent</td><td class="working">60%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 60%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Working - conversation is hard-coded for now.<BR>25Mar14</td></tr>
<tr><td> - Agents offer Missions</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr>
<tr><td> - Mission and Deadspace/Encounter creation</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr>
<tr><td> - Can Search Agents</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr>
<tr><td> - Can Search for character by agent</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Chats_-_Local_Corp_Custom_Private"> Large Scale Chat System (LSC)  </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:94%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:94%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>93.8</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Open/View Static Server Channels</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Join Static Server Channels</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - rClick Chat Menu</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Characters show up in Local</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Still Testing.<BR>24Nov17</td></tr>
<tr><td> - Chat in Local</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Characters show up in Corp</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Still Testing.<BR>24Nov17</td></tr>
<tr><td> - Chat in Corp</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working<BR>24Nov17</td></tr>
<tr><td> - Initiate Private Conversation Chat</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Few Odd Bugs<BR>7Sep14</td></tr>
<tr><td> - Create Private Chat Channels</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Joining Created Private Chat Channels</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Few Odd Bugs<BR>7Sep14</td></tr>
<tr><td> - Chat in Private Chat Channels</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Load Subscribed Chat Channels Upon Login</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Configure Private Chat Channels</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Few Odd Bugs<BR>7Sep14</td></tr>
<tr><td> - Leave Chat Channels</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Unsubscribe from Private Chat Channel</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Character and Corporation Lookup (via Search)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="EvE_Mail"> EvE Mail </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:40%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:40%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>40.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td><font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can Send Eve-mails</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working, some bugs<BR>7Sep14</td></tr>
<tr><td> - Can Receive Eve-mails</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 81%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working, some bugs<BR>7Sep14</td></tr>
<tr><td> - Can View Eve-mails</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 81%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working, some bugs<BR>7Sep14</td></tr>
<tr><td> - Can Reply-to Eve-mails</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 81%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working, some bugs<BR>7Sep14</td></tr>
<tr><td> - Can Send Eve-mails to Groups (like Corp)</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>UNKNOWN<BR>7Sep14</td></tr>
<tr><td> - Can Create new Private Mailing Lists</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken<BR>7Sep14</td></tr>
<tr><td> - Newly Created Private Mailing Lists Saved to DB/Restored from DB</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken<BR>7Sep14</td></tr>
<tr><td> - Can Send/Receive messages in new Private Mailing Lists</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken<BR>7Sep14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Item_Properties_Window"> Item Properties Window </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:95%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:95%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>95.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - View item properties</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - View item picture (orange eye button)</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Market_Details"> Market Details </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:80%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:80%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>80.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Placing Sell Orders</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Needs More Testing<BR>10Aug16</td></tr>
<tr><td> - Cancelling Sell Orders</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Needs More Testing<BR>10Aug16</td></tr>
<tr><td> - Modify Sell Orders</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Needs More Testing<BR>10Aug16</td></tr>
<tr><td> - Placing Buy Orders</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Needs More Testing<BR>10Aug16</td></tr>
<tr><td> - Cancelling Buy Orders</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Needs More Testing<BR>10Aug16</td></tr>
<tr><td> - Modify Buy Orders</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Needs More Testing<BR>10Aug16</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Assets_Window"> Assets Window </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:85%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:85%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>85.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - All Assets View</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - View Assets by Region</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - View Assets by Constellation</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - View Assets by Solar System</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - View Assets by Station</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Search Assets</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only<BR>6Mar14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Wallet_Window"> Wallet </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:25%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:25%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>25.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - View Journal Entries</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only<BR>6Mar14</td></tr>
<tr><td> - View Shares Entries</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only<BR>6Mar14</td></tr>
<tr><td> - View Orders Entries</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only<BR>6Mar14</td></tr>
<tr><td> - View Transactions Entries</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Working for Market Transactions and Trade.<BR>6Feb16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Contracts_Window"> Contracts </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:16.6%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:16.6%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>16.6</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Start Page</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 50%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working<BR>6Mar14</td></tr>
<tr><td> - Create Contracts</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working - cannot complete contract<BR>27July15</td></tr>
<tr><td> - Get contract info</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working<BR>6Mar14</td></tr>
<tr><td> - List our contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>6Mar14</td></tr>
<tr><td> - List available contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>6Mar14</td></tr>
<tr><td> - Delete contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>6Mar14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Map_System"> Map System </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:85%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:85%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>85.0</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can show Solar System map</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Can show/use Map Browser (F11)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Can search Solar Systems</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Can plot routes by searching systems and adding waypoints</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Can Get Satistics on Systems</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Most Aspects Working<BR>25Mar14</td></tr>
<tr><td> - Can Get Visited Systems</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Can Get Jumps Last Hour</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Can Get Pilots in Space</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr>
<tr><td> - Can Get Pilots Docked and Active.</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>6Mar14</td></tr>
<tr><td> - Can Get Kills / Kills last 24 Hours</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Can Get PodKills / PodKills last 24 Hours</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Can Get My Avalible Agents</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>Agents not implemented.<BR>6Mar14</td></tr>
<tr><td> - Can Get My Bookmarks</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Can Get My Assets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Sep14</td></tr>
<tr><td> - Can Get Active Cyno Fields</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>Cyno Generator not implemented.<BR>6Mar14</td></tr>
<tr><td> - Can Get Recent Sovernity Changes</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>Contested Sovernity not implemented.<BR>6Mar14</td></tr>
<tr><td> - Can Get Incursions</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>Incursions not implemented.<BR>6Mar14</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Bookmark_System"> Bookmark System </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:92%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:92%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>92.2</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Bookmark system</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - Bookmarks loaded from database at character login</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Bookmark a location in space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Bookmark a station, planet, moon, stargate</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Update a Bookmark</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Delete Bookmarks</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Copy Bookmark to Ship Cargo or Station Hangar</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Implemented, but *I* haven't been able to decode the Python from client.<BR>6Mar14</td></tr>
<p><br style="clear:both;" /></p>
<tr><td> - Create a Bookmark Folder</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Update a Bookmark Folder</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>30Dec14</td></tr>
<tr><td> - Delete a Bookmark Folder</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - Move Bookmark to a Bookmark Folder</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Overview"> Overview </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>&nbsp;</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Selected Item Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
<tr><td> - RClick on item in overview and interact</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Feb14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Ship_Management_in_Space"> Ship Management </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:73%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:73%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color="red"><b>73.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can Jettison items into space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>29Nov15</td></tr>
<tr><td> - Can open Jetcans and Add/Remove items within them</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>29Nov15</td></tr>
<tr><td> - Can open Containers in space and Add/Remove items within them</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>29Nov15</td></tr>
<tr><td> - Can eject from ship to pod</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>9Dec15</td></tr>
<tr><td> - Unpiloted ships stay in space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>9Dec15</td></tr>
<tr><td> - Can board ships in space from pod</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>9Dec15</td></tr>
<tr><td> - Can name your ship</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can activate Self-Destruct</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>22Aug15</td></tr>
<tr><td colspan=3>&nbsp;&nbsp;Capital Ship Management</td></tr>
<tr><td> - Can configure ship</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>7Sep14</td></tr>
<tr><td> - Can install clone</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>7Sep14</td></tr>
<tr><td> - Can store/retrieve ships</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>10Aug16</td></tr>
<tr><td> - Can store/retrieve items from Corporate Hangar</td><td class="working">50%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 50%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Cannot set Corp Roles<BR>10Aug16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Ship_Navigation_in_Space"> Ship Navigation (Destiny) </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:91%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:91%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>90.4</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Stargate Jump</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Orbit Object</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Hacked - Working very well<BR>15Jul16</td></tr>
<tr><td> - Follow Object</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Approach Object</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Keep Object at Distance</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Aligning to Object</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Warp to object on-grid</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Warp to object off-grid</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Dock to Station</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Undock from Station (with velocity)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Can travel routes using AutoPilot</td><td class="broken">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken - AP shuts off after every jump<BR>7Sep14</td></tr>
<tr><td> - Warp-in from random location at login</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Coded but not implemented<BR>10Aug16</td></tr>
<tr><td> - Warp-out to random location at logoff</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Coded but not implemented<BR>10Aug16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Ship_Combat_in_Space"> Ship Combat </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:99%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:97%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>99.2</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can Lock Target</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Can Activate Modules</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Modules affect target</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Can Orbit target</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Hacked - Working very well<BR>15Jul16</td></tr>
<tr><td> - Can Follow target</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Can Keep target at distance</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Module_Management"> Module Management (Rewrite)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>&nbsp;</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can drop modules on center of Fitting window (auto-insert into slots)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can drop modules on specific slots in Fitting window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can Online/Offline modules in Fitting window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can remove modules from Fitting window to cargo or hangar</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can add/remove charges to modules slots in Fitting window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can move modules from slot to slot in Fitting window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can Activate modules on ship in space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Activated modules on ship in space produce desired effect</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - Can De-activate modules on ship in space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
<tr><td> - De-activated modules on ship in space remove effect</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>10Aug16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Ship_Combat_in_Space"> Ship Module Groups </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:87%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:87%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>87.3</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Module Group</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Engineering</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Electronic</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Weapons - Turrents</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Weapons - Missiles</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Weapons - Other</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Code Only - Not Implemented<BR>10Aug16</td></tr>
<tr><td> - Shields</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Armor</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Hull</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - EWAR</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Propulsion</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - Mining</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
<tr><td> - Gang Assist</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Code Only - Fleets Not Implemented<BR>10Aug16</td></tr>
<tr><td> - Rigs</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27Jun17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Drones"> Drones </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:40.8%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:40.8%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>40.8</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Launch Drones</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00">
</td><td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table>
<td>Entity Creation and Launching Working - Needs More Testing<BR>21May16</td></tr>
<tr><td> - Scoop Drones to Drone Bay</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 85%; background:#00DB00">
</td><td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table>
<td>Working - Needs more testing<BR>21May16</td></tr>
<tr><td> - Scoop Drones to Cargo Bay</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 85%; background:#00DB00">
</td><td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table>
<td>Working - Needs more testing<BR>21May16</td></tr>
<tr><td> - Drone AI</td><td class="working">45%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 45%; background:#00DB00">
</td><td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table>
<td>Somewhat Working - Very Basic.  Can Pick targets, but wont engage or return.<BR>21May16</td></tr>
<tr><td> - Mining Drones</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Somewhat Working - Very Basic<BR>No Specific AI<BR>21May16</td></tr>
<tr><td> - Combat Drones</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Somewhat Working - Very Basic<BR>No Specific AI<BR>21May16</td></tr>
<tr><td> - Electronic Combat Drones</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Somewhat Working - Very Basic<BR>No Specific AI<BR>21May16</td></tr>
<tr><td> - Logistics Drones</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Somewhat Working - Very Basic<BR>No Specific AI<BR>21May16</td></tr>
<tr><td> - Engage</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Not Working - wont engage.<BR>21May16</td></tr>
<tr><td> - Return</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Not Working - wont return.<BR>21May16</td></tr>
<tr><td> - Recall to orbit</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Not Working - wont orbit.<BR>21May16</td></tr>
<tr><td> - Recall To Bay</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Not Working - wont return.<BR>21May16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="NPC_Combat_Spawning_AI"> NPC AI, Combat &amp; Spawning </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:65.6%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:65.6%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>65.6</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - NPC look for targets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - NPC engage when targeted (if idle)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - NPC approach found targets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - NPC lock/engage/orbit targets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - NPC notice new targets within sight range, and engage weakest</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Implemented - very buggy<BR>27July15</td></tr>
<tr><td> - NPC special actions/call for help/retreat</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>27July15</td></tr>
<tr><td> - NPC Dynamic spawning</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - NPC Static spawning</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only.<BR>27July15</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Science_and_Industry"> Science and Industry </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:45%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:45%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>45.5</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Research</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - few bugs.<BR>6Mar15</td></tr>
<tr><td> - Invention</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only<BR>6Mar15</td></tr>
<tr><td> - Reverse Engineering</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only<BR>6Mar15</td></tr>
<tr><td> - Ore Mining</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - few bugs.<BR>6Mar15</td></tr>
<tr><td> - Moon Mining</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>30Nov15</td></tr>
<tr><td> - Gas Mining</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Non-Working - very buggy.<BR>6Mar15</td></tr>
<tr><td> - Refining and Reprocessing</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>27July15</td></tr>
<tr><td> - Manufacturing</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>17Nov17</td></tr>
<tr><td> - R&amp;D using Agents</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>30Nov15</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Scan_and_Directional_Probing"> Scanning </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:37.5%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:37.5%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>37.5</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - System Scanning using Ship Sensors (Without Probes)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>21May16</td></tr>
<tr><td> - System Scanning with Probes</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Framework Only<BR>6Mar15</td></tr>
<tr><td> - Directional Scanning (using Ship Sensors)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>7Dec15</td></tr>
<tr><td> - Launch Probes from ship</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Implemented - very buggy<BR>27July15</td></tr>
<tr><td> - Move Probes in Space</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>6Mar15</td></tr>
<tr><td> - Change Probe Range</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>6Mar15</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Fleet_System"> Fleet </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:94%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:94%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>94.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Create Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Fleet management window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Set Fleet MOTD</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>1Dec17</td></tr>
<tr><td> - Fleet Warp</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Not Coded<BR>23Nov17</td></tr>
<tr><td> - View Fleet Composition</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Create/Delete Wings and Squads</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Name Wings and Squads</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Invite to Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Accept Fleet Invite</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Reject Fleet Invite</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Leave Fleet</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Working, but cannot rejoin or create fleet after leave<BR>1Dec17</td></tr>
<tr><td> - Kick Member</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Working, but cannot rejoin or create fleet after leave<BR>1Dec17</td></tr>
<tr><td> - Move fleet members</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Set commanders</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Set boosters</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Commanders provide bonuses to their members</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>1Dec17</td></tr>
<tr><td> - Boosters provide bonuses to their members</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>1Dec17</td></tr>
<tr><td> - Broadcast to fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Broadcast to system</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Broadcast to bubble</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Add Member to watch list</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>1Dec17</td></tr>
<tr><td> - Remove Member from watch list</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>1Dec17</td></tr>
<tr><td> - Fleet chat window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Advertise Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - View Availible Fleets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - Apply to Advertised Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>1Dec17</td></tr>
<tr><td> - View Fleet Applications</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Final Testing<BR>1Dec17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Planetary_Interaction"> Planetary Interaction System </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:73%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:73%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>73.3</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Planet Search</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Preliminary Implementation<BR>27Dec16</td></tr>
<tr><td> - Planet View</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Complete - Testing<BR>27Dec16</td></tr>
<tr><td> - Planet Resource Survey</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Complete - Testing<BR>27Dec16</td></tr>
<tr><td> - Install/Use Command Center </td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Complete - Testing<BR>27Dec16</td></tr>
<tr><td> - Creating Extractor Heads</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Complete - Testing<BR>27Dec16</td></tr>
<tr><td> - Install/Use Silo</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Complete - Testing<BR>27Dec16</td></tr>
<tr><td> - Creating logistic routes (links) on planet</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Complete - Testing<BR>27Dec16</td></tr>
<tr><td> - Installing Schematics</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Complete - Testing<BR>27Dec16</td></tr>
<tr><td> - Factory Automation (run schematics)</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Complete - Testing<BR>27Dec16</td></tr>
<tr><td> - Install/Use Space Ports</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Preliminary Implementation<BR>27Dec16</td></tr>
<tr><td> - Planetary Customs Office</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Preliminary Implementation<BR>27Dec16</td></tr>
<tr><td> - Launching Items to Customs Office</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Preliminary Implementation<BR>27Dec16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Corporation_Management"> Corporation Management </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:25%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:25%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>25.7</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Create a corporation</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>(but corp system is broken)<BR>1Feb16</td></tr>
<tr><td> - Manage Roles</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken - Needs Work<BR>7Sep14</td></tr>
<tr><td> - Corp Hangers and divisions</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken - Needs Work<BR>7Sep14</td></tr>
<tr><td> - Corp Wallet</td><td class="working">50%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 50%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Can Rename divisions, but cannot set access<BR>7Dec15</td></tr>
<tr><td> - Renting Office</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken - Needs Work<BR>7Sep14</td></tr>
<tr><td> - Declairing War</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken - Needs Work<BR>7Sep14</td></tr>
<tr><td> - Recruiting</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Broken - Needs Work<BR>7Sep14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Alliance_Management"> Alliance Management </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:1%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:1%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Creating Alliance</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>7Sep14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="POS_Operations"> Player Owned Station (POS) Operations </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:34%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:34%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>34.1</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Deploying Station Structures</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>(but corp system is broken)<BR>10Aug16</td></tr>
<tr><td> - Anchoring POS modules</td><td class="working">60%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 60%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working -  Needs more testing<BR>10Aug16</td></tr>
<tr><td> - Onlining Structures</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Code Only - cannot online structures yet.<BR>10Aug16</td></tr>
<tr><td> - Managing Stations</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Code Only - cannot online structures yet.<BR>10Aug16</td></tr>
<tr><td> - Interaction with POS modules</td><td class="working">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Code Only - cannot online structures yet.<BR>10Aug16</td></tr>
<tr><td> - Can store/retrieve ships from Ship Maintenance Arrays</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Skeleton Code Only - cannot online structures yet.<BR>10Aug16</td></tr>
<tr><td> - Can store/retrieve items from Corporate Hangar Arrays</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>COMPLETE<BR>(but corp system is broken)<BR>10Aug16</td></tr>
<tr><td> - Can Add/Remove Fuel for Control Tower</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Mostly Working - Needs Testing<BR>(but corp system is broken)<BR>10Aug16</td></tr>
<tr><td> - Accessing Station Cargo Holds</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Working - Can Access storage facilities for station, but not for offline structures.<BR>10Aug16</td></tr>
<tr><td> - Can Add/Remove Charges for Defensive Batteries</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED - cannot online structures yet.<BR>10Aug16</td></tr>
<tr><td> - Using weapons/factories/hangars</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED - cannot online structures yet.<BR>10Aug16</td></tr>
<tr><td> - Reinforcing Structures</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED - cannot online structures yet.<BR>10Aug16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Outpost_Construction_and_Management"> Outpost Construction and Managements </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:1%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:1%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color="red"><b>0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color="red"><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Outposts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>NOT IMPLEMENTED<BR>10Aug16</td></tr>
</table>
<p><br style="clear:both;" /></p>
</table><BR><BR>
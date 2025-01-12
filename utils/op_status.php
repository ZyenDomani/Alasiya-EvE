<tr><td class="content">
<h1><center><span class="mw-headline" id="Crucible">Alasiya's Crucible Server</span></center></h1>
<table style="background: transparent; width:100%"><tr>
<td style="text-align: center;"><h2>Estimated Progress of Listed Systems</h2></td></tr></table>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:70%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table></td></tr></table>
<table style="background: transparent; width:100%"><tr><td style="text-align: center;">
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:70%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>65.7%</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;"><BR><BR>
<font color=lightblue>This data is reflection of current Alasiya-EvE code status.<BR>If a system is NOT listed here, then it isnt implemented at all.<BR><BR>
<font color=green>Please Note:</font> <BR>
Items listed as "<font color=red>NOT IMPLEMENTED</font>" are not coded at all. <BR>
The server will not respond to these requests, you will get an error, and the server may even crash.  (this is ok, and expected)<BR><BR>
Items listed as "<font color=orange>Skeleton Code Only</font>" has a basic outline coded, but limited to no processing and/or returns. <BR>
The server may not respond or only partially respond to these requests. You may get an error from these requests, and the server may crash.<BR><BR>
Items listed as "<font color=yellow>Preliminary Implementation</font>" have a first draft of their specific code, possibably a step or two above skeleton code, and may not function properly or completely. <BR>
There will be some response, like opening windows, but *should not* generate an error or crash the server.</font><BR><BR><BR>
<font color=magenta>Last Update:</font>&nbsp;&nbsp;&nbsp;<font color=green>11 January 2025</font><BR>
<font color=magenta>Page Status:</font>&nbsp;&nbsp;&nbsp;<font color=red>Mostly Current</font> - <font color=yellow>Being Reviewed and Updated</font><BR>
<font color=magenta>Incomplete Data:</font>&nbsp;&nbsp;&nbsp;<font color=brown>Anything not listed as (<font color=green>Current</font>) is pending review.</font><BR><BR>
<a href="https://scan.coverity.com/projects/zyendomani-alasiyaeve">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/27332/badge.svg"/>
</a>&nbsp;&nbsp;&nbsp;&nbsp;
<a href="https://www.codefactor.io/repository/github/zyendomani/alasiya-eve">
  <img src="https://www.codefactor.io/repository/github/zyendomani/alasiya-eve/badge?s=633e0a6e1d656f3e4783924a400a34261abe49f4" alt="CodeFactor" />
</a>
  </table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="System_Foundation_Fundamentals"> System Foundation Fundamentals (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:66%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:66%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>65.8</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;"><tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td><td style="width: 26%;">Comments</td></tr>
<tr><td> - Stable, Efficient Server Code</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Constantly in Progress - monitoring & adjusting code for faster runtime. most segfaults eliminated<BR>Been working this code for >10y<BR>4Feb23</td></tr>
<tr><td> - Memory Management<BR>
<font color=green>InventoryItem and PhotoService are only classes with accurate MemMgmt</font></td><td class="working">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>Progress Restarted - Ongoing</font><BR><font color=orange>Rewrote Py* Object ref counting code. Working better, but still incomplete</font><BR>7Jan25</td></tr>
<tr><td> - Efficient, Accurate Packet Deciphering<BR><font color=blue>NOTE: at this point, "Efficient" is a misnomer.</font></td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>In Progress - Most packets understood and correctly processed<BR>4Feb23</td></tr>
<tr><td> - Mutexes</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR><font color=orange>Recently found very elusive data race.<BR>Not solved yet.</font><BR>8Mar23</td></tr>
<tr><td> - Eliminate all compiler Warnings from the Alasiya EVEmu solution</td><td class="working">60%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 60%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=pink>In Progress</font><BR><font color=olive>Enabling all compiler warnings gives 9.6Mb log file of errors</font><BR>24Mar23</td></tr>
<tr><td> - GM commands<BR>Most commands need complete rewrite or major update</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=magenta>Needs Work</font><BR>4Feb23</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Information"> Item Information Windows (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:95%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:90%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>95.0</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Get Item Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>10Feb20</td></tr>
<tr><td> - Get Ship Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>10Feb20</td></tr>
<tr><td> - Get Skill Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>10Feb20</td></tr>
<tr><td> - Get Certification Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>10Feb20</td></tr>
<tr><td> - Get Character Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table>
<td><font color=lime>Final Testing</font><BR>10Feb20</td></tr>
<tr><td> - Get NPC Corporation Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>10Feb20</td></tr>
<tr><td> - Get Player Corporation Info</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>10Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Account_and_Character_Management"> Account and Character Management (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:96%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table></td></tr></table></div></div>
<div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td><table style="background: transparent; width:96%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>96.3</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Account Login</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>15Feb14</td></tr>
<tr><td> - Character Creation</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>15Feb14</td></tr>
<tr><td> - Character Deletion</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>3Mar19</td></tr>
<tr><td> - Character entrance to last location</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Character Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>17Nov17</td></tr>
<tr><td> - Open Skills Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Open Certificate Window and Planner</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Open Decorations Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30May19</td></tr>
<tr><td> - Open Attributes Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Neural Remap</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00">
</td><td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Open Augmentations Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Open Jump Clones Window</td><td class="broken">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR><font color=red>Jump Clones Not Implemented</font><BR>1Feb14</td></tr>
<tr><td> - Open Bio Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Open Employment History Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>4Apr14</td></tr>
<tr><td> - Open Standings Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>Standings mostly Implemented (see below)<BR>1Jan25</td></tr>
<tr><td> - Open Security Status Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>SecStatus mostly Implemented.<BR>1Jan25</td></tr>
<tr><td> - Open Kill Rights Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR><font color=red>Kill Rights Not Implemented</font><BR>1Feb14</td></tr><tr>
<td> - Open Combat Log Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>Combat Log partially Implemented<BR>17July17</td></tr>
<td> - KillMail</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>17July17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Skills_-_Certificates"> Skills &amp; Certificates (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:97%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:90%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>97.7</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td><font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Browse Skills to train</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar14</td></tr>
<tr><td> - Add skills to queue</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar14</td></tr>
<tr><td> - Filter skills that don't fit in queue time remaining</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar14</td></tr>
<tr><td> - Apply changes to skill queue</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar14</td></tr>
<tr><td> - Pause skill queue</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar14</td></tr>
<tr><td> - Re-Start skill queue after pause</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>17Nov17</td></tr>
<tr><td> - Skill training time</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar14</td></tr>
<tr><td> - Right-click add skill to queue from Skills Window in Character Window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar14</td></tr>
<tr><td> - Certificate Awarding<BR><font color=orange>Recently found some major errors with requirements for certs</font></td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR><font color=yellow>Few Odd Bugs</font><BR>8Mar23</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Standings"> Standings (<font color=yellow>needs review</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:23%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:23%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>22.3</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td><font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - <font color=lime>Faction Standings (NPC Alliances)</font> &nbsp;&nbsp;--&nbsp;&nbsp; <font color=yellow>These standings cannot be modified.</font></td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar14</td></tr>
<tr><td> - <font color=lime>CONCORD Standings (Security Rating)</font>&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by npc/pc kills</font></td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>3Feb23</td></tr>
<tr><td> - Agent to Character&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by missions</font></td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>3Feb23</td></tr>
<tr><td> - Agent to Player Corp&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by missions</font></td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Agents respond to Character standings (personal, corp, ally)</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>2Feb20</td></tr>
<tr><td> - NPC Corp to Character&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by missions and faction kills</font></td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>2Feb20</td></tr>
<tr><td> - NPC Corp to Player Corp&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by missions and faction kills</font></td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>2Feb20</td></tr>
<tr><td> - Character to Character&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by PnP window</font></td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>2Feb20</td></tr>
<tr><td> - Character to Player Corp&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by PnP window</font></td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Player Corp to Character&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by Corp Management window</font></td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Player Corp to Player Corp&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by Corp Management window</font></td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Alliance to Player Corp&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by Corp Management window</font></td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Alliance to Alliance&nbsp;&nbsp;--&nbsp;&nbsp; <font color=green>Changed by Corp Management window</font></td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="NPC_Station_Services"> NPC Station Services (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:83%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:83%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>82.7</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Right-click Menu in Station</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>10Aug16</td></tr>
<tr><td> - In-station Guests List</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>10Aug16</td></tr>
<tr><td> - Can repackage ships/items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Can assemble ships and items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>21Feb14</td></tr>
<tr><td> - Can eject from ship to pod</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Nov15</td></tr>
<tr><td> - Can make active ships</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Nov15</td></tr>
<tr><td> - Agent Services</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>13Nov18</td></tr>
<tr><td> - Trade Services</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR><font color=red>corp trade not coded.</font><BR>27Jun17</td></tr>
<tr><td> - Repair Services</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>19Nov18</td></tr>
<tr><td> - Insurance Services</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>22Aug15</td></tr>
<tr><td> - Bounty Services</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>22Aug15</td></tr>
<tr><td> - Market Services</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR>
<font color=red>Corp Buy/Sell Incomplete</font><BR>10Dec18</td></tr>
<tr><td> - Medical Services</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR><font color=yellow>Few Odd Bugs</font><BR>30Dec14</td></tr>
<tr><td> - LP Services</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>6Mar14</td></tr>
<tr><td> - Clone upgrade</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
<tr><td> - Clone transfer</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>7Sep14</td></tr>
<tr><td> - Jump clone installation</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>6Mar14</td></tr>
<tr><td> - Items window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>21Feb14</td></tr>
<tr><td> - Can merge/stack/split items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Can trash items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Can open/close containers</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Can move items into/out of containers</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
<tr><td> - Can inject skills from items</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Feb14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Agents"> Agents (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:63%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:63%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>63.1</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Agents Tab</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>19Nov18</td></tr>
<tr><td> - Can initiate conversation with agent</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>19Nov18</td></tr>
<tr><td> - Agents offer Missions</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>19Nov18</td></tr>
<tr><td> - Can View Mission Offer</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>19Nov18</td></tr>
<tr><td> - Can Accept Mission Offer</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>19Nov18</td></tr>
<tr><td> - Can Complete Mission Offer</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>19Nov18</td></tr>
<tr><td> - Can Search Agents</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>19Nov18</td></tr>
<tr><td> - Can Search for character by agent</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>19Nov18</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2><span class="mw-headline" id="Missions"> Missions (<font color=green>Current</font>)</span></h2>&nbsp;&nbsp;&nbsp;&nbsp;See forums for details on types and offers
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:15%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:15%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>13.5</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Missions Give Rewards (isk/standings)</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=pink>Testing</font><BR>19Nov18</td></tr>
<tr><td> - Missions Give LP</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>19Nov18</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Basic Mission Types</h3></font></td></tr>
<tr><td> - <font color=lime>Courier Missions</font><BR>Simple delivery where you are requested to move items from one location to another.<BR>
Item(s) to deliver are supplied by Agent.<BR>No standing loss from opposing faction.</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Mining Missions</font> - 2 Types. <BR>Mine specific ore at specific location.<BR>
Supply agent with ore or minerals.</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Security Missions</font> - 2 Types.<BR>Destroy ships at given location in space.<BR>
Destroy ships in deadspace pockets using acceleration gates.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Trade Missions</font><BR>Provide item(s) to specific station.<BR>Can be either crafted or bought. <BR> No standing loss from opposing faction.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Special Mission Types</h3></font></td></tr>
<tr><td> - <font color=lime>Tutorial Missions</font><BR> Missions for beginning pilots to learn game basics.<BR>
<font color=yellow>On Hold...May not be implemented on Alasiya.</font></td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Storyline Missions</font><BR>Offered from Special agent after completion of 16 same-level missions for same faction.<BR> Repeatable.</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>8Mar23</td></tr>
<tr><td> - <font color=lime>Research Missions</font><BR>Special Missions given by Research Agents during ongoing research.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Data Missions</font><BR>Turn in pirate tags for standings.<BR>Higher Agents may give basic missions for higher rewards.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Cosmos Missions</font><BR> Mission Chains given from CONCORD agents.  Can only be completed once. <BR>Expiration, Failure or Denial forever blocks giving agent.<BR><font color=yellow>On Hold...May not be implemented on Alasiya.</font></td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Anomic Missions</font><BR> Given by L4 Security Agents. Tougher than 'regular' L4 missions.  Can be declined without penalty.  Ship Restrictions.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Arc Missions</font><BR> Large Mission Chains broken into chapters.  7 total Epic Arcs.  Can be repeated every 3 months.<BR>
<font color=yellow>On Hold...May not be implemented on Alasiya.</font></td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Burner Missions</font><BR>Can be offered from any Agent. Can be declined without penalty.  Ship Restrictions.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Nov18</td></tr>
<tr><td> - <font color=lime>Unsorted Missions</font><BR> Custom missions separate from other categories.</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>19Nov18</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Chats_-_Local_Corp_Custom_Private"> LSC - Large Scale Chat System (<font color=pink>Rewrite on hold</font>) </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:68%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:68%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>68.1</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Open/View Static Server Channels</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Join Static Server Channels</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - rClick Chat Menu</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Characters show up in Local</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>24Nov17</td></tr>
<tr><td> - Chat in Local</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
<tr><td> - Characters show up in Corp</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR>24Nov17</td></tr>
<tr><td> - Chat in Corp</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR>24Nov17</td></tr>
<tr><td> - Initiate Private Conversation Chat</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>2Feb20</td></tr>
<tr><td> - Create Private Chat Channels</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>2Feb20</td></tr>
<tr><td> - Joining Created Private Chat Channels</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>2Feb20</td></tr>
<tr><td> - Chat in Private Chat Channels</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>2Feb20</td></tr>
<tr><td> - Load Subscribed Chat Channels Upon Login</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Configure Private Chat Channels</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>2Feb20</td></tr>
<tr><td> - Leave Chat Channels</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Unsubscribe from Private Chat Channel</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>2Feb20</td></tr>
<tr><td> - Character and Corporation Lookup (via Search)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="EvE_Mail"> EvE Mail (<font color=pink>Rewrite on hold</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:20%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:20%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>1.6</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td><font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - System-Generated mail <BR>(insurance, kills, corp notifications, etc.)</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Characters</h3></font></td></tr>
<tr><td> - Can Send Eve-mails</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Still Buggy</font><BR>7Sep14</td></tr>
<tr><td> - Can Receive Eve-mails</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Still Buggy</font><BR>7Sep14</td></tr>
<tr><td> - Can View Eve-mails</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Still Buggy</font><BR>7Sep14</td></tr>
<tr><td> - Can Reply-to Eve-mails</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Still Buggy</font><BR>7Sep14</td></tr>
<tr><td> - Can Send Eve-mails to Groups (like Corp)</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Can Create new Private Mailing Lists</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Private Mailing Lists Saved to DB/Restored from DB</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Can Send/Receive messages in new Private Mailing Lists</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporation Mail</h3></font></td></tr>
<tr><td> - Can Send Eve-mails</td><td class="working">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Can Receive Eve-mails</td><td class="working">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Can View Eve-mails</td><td class="working">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Can Reply-to Eve-mails</td><td class="working">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Can Send Eve-mails to Groups (like Corp)</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Can Create new Private Mailing Lists</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Private Mailing Lists Saved to DB/Restored from DB</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Can Send/Receive messages in new Private Mailing Lists</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Belt_Manager"> Calendar (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:69%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:69%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>68.5</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Open Calendar</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td> - View Events</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td> - Respond to Event</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td> - Edit Response to Event</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>31Oct20</td></tr>
<tr><td> - View Event Responses</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Personal </h3></font></td></tr>
<tr><td> - Create Event</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td> - Edit Event</td><td class="working">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>31Oct20</td></tr>
<tr><td> - Delete Event</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporate </h3></font></td></tr>
<tr><td> - Create Event</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td> - Edit Event</td><td class="working">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>31Oct20</td></tr>
<tr><td> - Delete Event</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Alliance </h3></font></td></tr>
<tr><td> - Create Event</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR><font color=pink>Alliances partially implemented</font><BR>31Oct20</td></tr>
<tr><td> - Edit Event</td><td class="working">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>31Oct20</td></tr>
<tr><td> - Delete Event</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR><font color=pink>Alliances partially implemented</font><BR>31Oct20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Market_Details"> Market Details (<font color=pink>Rewrite on hold</font> - <font color=yellow>needs review</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:30%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:30%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>40.4</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Price History</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>11Dec18</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Characters</h3></font></td></tr>
<tr><td> - View Items on Market</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>11Dec18</td></tr>
<tr><td> - View Item Details (Price History, Buy/Sell orders)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>11Dec18</td></tr>
<tr><td> - Buy Items on Market (auto-pick seller)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>11Dec18</td></tr>
<tr><td> - Buy Items on Market in Specific Location</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>11Dec18</td></tr>
<tr><td> - Placing Buy Orders</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>11Dec18</td></tr>
<tr><td> - Cancelling Buy Orders</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>11Dec18</td></tr>
<tr><td> - Modify Buy Orders</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>11Dec18</td></tr>
<tr><td> - Placing Sell Orders</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>11Dec18</td></tr>
<tr><td> - Cancelling Sell Orders</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>11Dec18</td></tr>
<tr><td> - Modify Sell Orders</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 40%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>11Dec18</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporations</h3></font></td></tr>
<tr><td> - Buy Items on Market (auto-pick seller)</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>26May19</td></tr>
<tr><td> - Buy Items on Market in Specific Location</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>26May19</td></tr>
<tr><td> - Placing Buy Orders</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>26May19</td></tr>
<tr><td> - Cancelling Buy Orders</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>26May19</td></tr>
<tr><td> - Modify Buy Orders</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>26May19</td></tr>
<tr><td> - Placing Sell Orders</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>26May19</td></tr>
<tr><td> - Cancelling Sell Orders</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>26May19</td></tr>
<tr><td> - Modify Sell Orders</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>26May19</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Trader Joe (MarketBot)</h3></font></td></tr>
<tr><td> - Watches for Active Systems for trade</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30Nov21</td></tr>
<tr><td> - Buy Items on Market</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30Nov21</td></tr>
<tr><td> - Placing Buy Orders</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30Nov21</td></tr>
<tr><td> - Sell Items on Market</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30Nov21</td></tr>
<tr><td> - Placing Sell Orders</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30Nov21</td></tr>
<tr><td> - Fleet travels using Civilian Manager system</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30Nov21</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Assets_Window"> Assets Window (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:40%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:86%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>85.8</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Characters</h3></font></td></tr>
<tr><td> - All Assets View</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - View Assets by Region</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - View Assets by Constellation</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - View Assets by Solar System</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - View Assets by Station</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Search Assets</td><td class="working">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>6Mar14</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporations</h3></font></td></tr>
<tr><td> - All Assets View</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>26May19</td></tr>
<tr><td> - View Assets by Region</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>26May19</td></tr>
<tr><td> - View Assets by Constellation</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>26May19</td></tr>
<tr><td> - View Assets by Solar System</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>26May19</td></tr>
<tr><td> - View Assets by Station</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>26May19</td></tr>
<tr><td> - Search Assets</td><td class="working">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>26May19</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Wallet_Window"> Wallet (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:99%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:90%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>99.5</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Characters</h3></font></td></tr>
<tr><td> - View Journal Entries</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec17</td></tr>
<tr><td> - View Shares Entries</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec17</td></tr>
<tr><td> - View Orders Entries</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec17</td></tr>
<tr><td> - View Transactions Entries</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec17</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporations</h3></font></td></tr>
<tr><td> - View Journal Entries</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>30May19</td></tr>
<tr><td> - View Shares Entries</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>30May19</td></tr>
<tr><td> - View Orders Entries</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>30May19</td></tr>
<tr><td> - View Transactions Entries</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>30May19</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Contracts_Window"> Contracts (<font color=red>Needs Complete Rewrite</font>) </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:15%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:15%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>15.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Characters</h3></font></td></tr>
<tr><td> - Start Page</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working<BR>6Mar14</td></tr>
<tr><td> - Create Contracts</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working - cannot complete contract<BR>27July15</td></tr>
<tr><td> - Get contract info</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working<BR>6Mar14</td></tr>
<tr><td> - List our contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>6Mar14</td></tr>
<tr><td> - List available contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>6Mar14</td></tr>
<tr><td> - Delete contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>6Mar14</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporations</h3></font></td></tr>
<tr><td> - Start Page</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working<BR>6Mar14</td></tr>
<tr><td> - Create Contracts</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working - cannot complete contract<BR>27July15</td></tr>
<tr><td> - Get contract info</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td>Partially Working<BR>6Mar14</td></tr>
<tr><td> - List our contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>6Mar14</td></tr>
<tr><td> - List available contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>6Mar14</td></tr>
<tr><td> - Delete contracts</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>6Mar14</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Map_System"> Map System (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:89%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:89%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>88.8</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can show Solar System map</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Can show/use Map Browser (F11)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Can search Solar Systems</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Can plot routes by searching systems and adding waypoints</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Can Get Statistics on Systems</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>25Mar14</td></tr>
<tr><td> - Can Get Visited Systems</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
<tr><td> - Can Get Jumps Last Hour</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
<tr><td> - Can Get Pilots in Space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>26Nov18</td></tr>
<tr><td> - Can Get Pilots Docked and Active.</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>26Nov18</td></tr>
<tr><td> - Can Get Kills / Kills last 24 Hours</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
<tr><td> - Can Get PodKills / PodKills last 24 Hours</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
<tr><td> - Can Get My Available Agents</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Nov18</td></tr>
<tr><td> - Can Get My Bookmarks</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
<tr><td> - Can Get My Assets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>7Sep14</td></tr>
<tr><td> - Can Get My CorpMembers in Space</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>6Mar14</td></tr>
<tr><td> - Can Get Active Cyno Fields</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR><font color=red>Cyno Generator Not Implemented</font><BR>26Nov18</td></tr>
<tr><td> - Can Get Recent Sovernity Changes</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR><font color=red>Contested Sovereignty Not Implemented</font><BR>6Mar14</td></tr>
<tr><td> - Can Get Incursions</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR><font color=red>Incursions Not Implemented</font><BR>6Mar14</td></tr></table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Bookmark_System"> Bookmark System  (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:94%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:94%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>94.4</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;
</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Character Bookmarks</h3></font></td></tr>
<tr><td> - Bookmark system</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>2Feb20</td></tr>
<tr><td> - Bookmarks loaded from database at character login</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Bookmark a location in space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Bookmark a station, planet, moon, stargate</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Update a Bookmark</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Delete Bookmarks</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Copy Bookmark to Ship Cargo or Station Hangar</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Feb20</td></tr>
<p><br style="clear:both;" /></p>
<tr><td> - Create a Bookmark Folder</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Update a Bookmark Folder</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Dec14</td></tr>
<tr><td> - Delete a Bookmark Folder</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>2Feb20</td></tr>
<tr><td> - Move Bookmark to another Folder</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>2Feb20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporation Bookmarks</h3></font></td></tr>
<tr><td> - Update a Character Bookmark</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Feb20</td></tr>
<tr><td> - Update a Corp Bookmark</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Feb20</td></tr>
<tr><td> - Delete Bookmarks</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Feb20</td></tr>
<tr><td> - Copy Bookmark to Ship Cargo or Station Hangar</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Feb20</td></tr>
<tr><td> - Create a Bookmark Folder</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Feb20</td></tr>
<tr><td> - Update a Bookmark Folder</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Feb20</td></tr>
<tr><td> - Delete a Bookmark Folder</td><td class="working">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30May19</td></tr>
<tr><td> - Move Bookmark to another Folder</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="fxSystem"> Effects System (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:83%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:83%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>83.1</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - <font color=lime>Parsing Effects</font><BR>&nbsp;&nbsp;&nbsp;This is decoding effects data before processing can occur</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>17Feb20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Fx Source Processing - Item applying the effect</h3></font></td></tr>
<tr><td> - <font color=lime>Process Self Effects</font><BR>&nbsp;&nbsp;&nbsp;Source is Ship (T1, T2), Module or Charge</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>10Feb20</td></tr>
<tr><td> - <font color=lime>Process Skill Effects</font><BR>&nbsp;&nbsp;&nbsp;Source is Skill, Implant or Booster</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR><font color=red>Boosters not implemented</font><BR>11Jan25</td></tr>
<tr><td> - <font color=lime>Process Ship Effects</font><BR>&nbsp;&nbsp;&nbsp;Source is Ship Subsystem (T3 ships only)</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>2Feb20</td></tr>
<tr><td> - <font color=lime>Process Group Effects</font><BR>&nbsp;&nbsp;&nbsp;Source is not a source per se, this defines the target selection requirements</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>19Jun19</td></tr>
<!--
<tr><td> - <font color=lime>Process Gang Effects</font><BR>&nbsp;&nbsp;&nbsp;Fleet Booster Skills</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>Actual Fleet Bonuses are hard-coded and do not process thru Fx System.<BR>19Jun19</td></tr>
<tr><td> - <font color=lime>Process Owner Effects</font><BR>Owner Source Unknown at this time</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Jun19</td></tr>
<tr><td> - <font color=lime>Process Target Effects</font><BR>&nbsp;&nbsp;&nbsp;Target Source Unknown at this time</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font><BR><font color=olive>Testing</font><BR>19Jun19</td></tr>
// -->
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Fx Target Processing - These vary depending on Source</h3></font></td></tr>
<tr><td> - <font color=lime>Apply Processed Effects to Self</font><BR>&nbsp;&nbsp;&nbsp;Source Item or a Location for Group Effects</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>17Feb20</td></tr>
<tr><td> - <font color=lime>Apply Processed Effects to Character</font><BR>&nbsp;&nbsp;&nbsp;Character or Character Skills</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>7Feb20</td></tr>
<tr><td> - <font color=lime>Apply Processed Effects to Ship</font><BR>&nbsp;&nbsp;&nbsp;Character's Ship</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>17Feb20</td></tr>
<tr><td> - <font color=lime>Apply Processed Effects to Target</font><BR>&nbsp;&nbsp;&nbsp;Ship's current target, either ship or structure.</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>19Jun19</td></tr>
<tr><td> - <font color=lime>Apply Processed Effects to Other</font><BR>&nbsp;&nbsp;&nbsp;Character's ship for Skill Source.  Specific Module for Charge Source.</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>17Feb20</td></tr>
<tr><td> - <font color=lime>Apply Processed Effects to Charge</font><BR>&nbsp;&nbsp;&nbsp;Loaded Charge on a Module or Module Group</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>17Feb20</td></tr>
<!--
<tr><td> - <font color=lime>Apply Processed Effects to Area</font><BR>&nbsp;&nbsp;&nbsp;Area Target Unknown at this time</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>19Jun19</td></tr>
// -->
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Ship_Management"> Ship Management (<font color=yellow>needs review</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:76%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:76%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> &#160;<font color=red><b>76.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right">
<tr><td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can Jettison items into space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>29Nov15</td></tr>
<tr><td> - Can open Jetcans and manipulate items within them</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>29Nov15</td></tr>
<tr><td> - Can open Containers in space and manipulate items within them</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>29Nov15</td></tr>
<tr><td> - Can eject from ship to pod</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>9Dec15</td></tr>
<tr><td> - Can Link Weapons</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>1Dec18</td></tr>
<tr><td> - Can AutoLink Weapons Using GroupAllWeapons button</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>1Dec18</td></tr>
<tr><td> - Can Load Linked Weapons<BR><font color=lime>NOTE: Linked Weapons Load as a group</font></td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>1Dec18</td></tr>
<tr><td> - Can Unload Linked Weapons<BR><font color=lime>NOTE: Linked Weapons UnLoad as a group</font></td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>1Dec18</td></tr>
<tr><td> - Can Unlink Weapons<BR><font color=lime>NOTE: Linked Weapons will not Unload Charges when Unlinked</font></td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>1Dec18</td></tr>
<!--
<tr><td> - Charges Stack in cargo while in space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec18</td></tr>
-->
<tr><td> - Unpiloted ships stay in space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>9Dec15</td></tr>
<tr><td> - Can board ships in space from pod</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>9Dec15</td></tr>
<tr><td> - Can name your ship</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>10Aug16</td></tr>
<tr><td> - Can use specialized bays on ships that have them</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>10Aug17</td></tr>
<tr><td> - Can activate Self-Destruct</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>2Feb20</td></tr>
<tr><td> - Modules Generate Heat when overloaded</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>2Feb20</td></tr>
<tr><td> - Module racks Dissipate Heat</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>2Feb20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;T3 Ship Management</h3></font></td></tr>
<tr><td> - Fit Subsystems</td><td class="broken">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>17Mar23</td></tr>
<tr><td> - Remove/Replace Subsystems</td><td class="broken">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>17Mar23</td></tr>
<tr><td> - Fit Modules</td><td class="broken">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>17Mar23</td></tr>
<tr><td> - Remove/Replace Modules</td><td class="broken">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>17Mar23</td></tr>
<tr><td> - Character loses skills when Ship is killed.</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>8Mar23</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Capital Ship Management</h3></font></td></tr>
<tr><td> - Can configure ship</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>7Sep14</td></tr>
<tr><td> - Can install clone</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>7Sep14</td></tr>
<tr><td> - Can store/retrieve ships</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>2Feb20</td></tr>
<tr><td> - Can store/retrieve items from Corporate Hangar</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>1Dec18</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Ship_Fittings"> Ship Fittings Manager (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:10%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:10%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>10.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Characters</h3></font></td></tr>
<tr><td> - Save Fittings</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Retrieve Saved Fittings</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Delete Fittings</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Update Fittings</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporations</h3></font></td></tr>
<tr><td> - Save Fittings</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Retrieve Saved Fittings</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Delete Fittings</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td> - Update Fittings</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Ship_Navigation"> Ship Navigation (<font color=brown">Destiny</font> - <font color=cyan>Movement Rewrite</font> - <font color=yellow>In Progress</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:84%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:84%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>83.8</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Stargate Jump</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Orbit Object</td><td class="broken">25%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 25%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Hacked - Horrible</font><BR>28Feb23</td></tr>
<tr><td> - Follow Object</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Approach Object</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Keep Object at Distance</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Aligning to Object</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Warp to object on-grid</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Warp to object off-grid</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Dock to Station</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Undock from Station (with velocity)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - Can travel routes using AutoPilot</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font> - <font color=cyan>AP shuts off after every jump</font><BR>7Sep14</td></tr>
<tr><td> - Warp-in from random location at login</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=red>NOT IMPLEMENTED</font><BR>10Aug16</td></tr>
<tr><td> - Warp-out to random location at logoff</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=red>NOT IMPLEMENTED</font><BR>10Aug16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Combat"> Combat (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:90%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:90%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>90.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can Lock Target</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Can Activate Modules</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Modules affect target</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Can Orbit target</td><td class="broken">25%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 25%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Hacked - Horrible</font><BR>8Mar23</td></tr>
<tr><td> - Can Follow target</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Can Keep target at distance</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Pilot moved to pod when ship destroyed</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Dec18</td></tr>
<tr><td> - Pilot moved to clone (at clone's location) when pod destroyed</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>2Dec18</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Module_Management"> Module Management (<font color=cyan>Rewrite</font> - <font color=yellow>needs review</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:83%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:83%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>83.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Can drop modules on ship in Fitting window (auto-insert into slots)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>10Aug16</td></tr>
<tr><td> - Can drop modules on specific slots in Fitting window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>10Aug16</td></tr>
<tr><td> - Can Online/Offline modules in Fitting window</td><td class="working">60%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 60%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Can Exploit</font> - <font color=violet>Needs Update</font><BR>25Feb20</td></tr>
<tr><td> - Can remove modules from Fitting window to cargo or hangar</td><td class="working">60%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 60%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Can Exploit</font> - <font color=violet>Needs Update</font><BR>25Feb20</td></tr>
<tr><td> - Can add/remove charges to modules slots in Fitting window</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Feb20</td></tr>
<tr><td> - Can move modules from slot to slot in Fitting window</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Feb20</td></tr>
<tr><td> - Can Activate modules on ship (in space only)</td><td class="working">60%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 60%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Can Exploit</font> - <font color=violet>Needs Update</font><BR>25Feb20</td></tr>
<tr><td> - Activated modules on ship in space produce desired effect</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Feb20</td></tr>
<tr><td> - Activated modules produce Graphics Effects<BR>GFX methods rewrite</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>8Mar23</td></tr>
<tr><td> - Can De-activate modules on ship in space</td><td class="working">60%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 60%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Can Exploit</font> - <font color=violet>Needs Update</font><BR>25Feb20</td></tr>
<tr><td> - De-activated modules on ship in space remove effect</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Feb20</td></tr>
<tr><td> - Can Overload modules (and they work)</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>12Jun21</td></tr>
<tr><td> - Can De-activate Overloaded modules</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>12Jun21</td></tr>
<tr><td> - Overloaded modules damage bank from heat</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Needs More Testing</font><BR>12Jun21</td></tr>
<tr><td> - Can repair damaged modules (using nanite paste)</td><td class="working">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>10Jan19</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Ship_Modules"> Ship Module Groups (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:87%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:87%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>87.3</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Module Group</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Engineering</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Electronic</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Weapons - Turrets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Weapons - Missiles</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Weapons - Other</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>10Aug16</td></tr>
<tr><td> - Shields</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Armor</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Hull</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - EWAR</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Propulsion</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>17Nov17</td></tr>
<tr><td> - Mining</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
<tr><td> - Gang Assist</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>10Aug16</td></tr>
<tr><td> - Rigs</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27Jun17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Drones"> Drones (<font color=cyan>Rewrite in progress</font> - <font color=green>Current</font>) </span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:18%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:18%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>18.1</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Launch Drones</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 90%; background:#00DB00">
</td><td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td> - Scoop to Cargo</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td> - Scoop to Drone Bay</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>31Oct20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Drone AI </h3></font></td></tr>
<tr><td><font color=lime>Passive/Aggressive</font>   (toggle)<BR><font color=teal>Passive</font>: attack only upon command.<BR>
<font color=teal>Aggressive</font>: attack any valid target.</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font> - <font color=orange>Very Basic.</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Attack and Follow</font> <font color=yellow>(fighters only)</font><BR>
Engage the first target of the assigned ship.<BR>Will chase target in warp.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Assist</font>   (passive/aggressive setting ignored)<BR>Drone will engage assigned ship's target once an offensive module is activated on that target. <BR>Drone will continue attack until itself, target or assigned ship is popped.</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Guard</font><BR>Drone will retaliate towards any threat to assigned ship until commanded otherwise.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Mine</font><BR>Drone will mine assigned target until canceled or full, then drop mined ore into assigned ship's hold.</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Focus Fire</font><BR>All drones fire upon (or mine) same target till destroyed or commanded otherwise.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Drone Commands </h3></font></td></tr>
<tr><td><font color=lime>Attack</font><BR>Engage the first target of the assigned ship.</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font> - <font color=orange>Very Basic.</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Assist</font><BR>Engage anything assigned ship activates offensive module on. (passive/aggressive setting ignored)</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Guard</font><BR> Retaliate towards any threat to assigned ship.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Mine</font><BR>Mine assigned asteroid for one cycle, then return and orbit.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Mine Repeatedly</font><BR>Mine assigned asteroid until depleted or commanded otherwise.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Abandon</font><BR>Relinquish control and ownership of drone.</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Return and orbit</font><BR>Should I explain this?</td><td class="working">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Return To Drone Bay</font><BR>To Be Determined.</td><td class="working">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>2Feb20</td></tr>
<tr><td><font color=lime>Reconnect to Lost Drones</font><BR>Try to reestablish communication with drones previously abandoned or went past ship's drone control range.</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="NPC_Combat_Spawning_AI"> NPC AI, Combat &amp; Spawning (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:84%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:84%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>83.6</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - NPC look for targets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - NPC engage when targeted (if idle)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - NPC approach found targets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - NPC lock/engage/orbit targets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>27July15</td></tr>
<tr><td> - NPC notice new targets within sight range, and engage weakest</td><td class="working">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>2Feb20</td></tr>
<tr><td> - NPC special actions/call for help/retreat</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Rats warp into Asteroid fields</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Still Glitchy</font><BR>2Feb23</td></tr>
<tr><td> - Rats warp out of Asteroid fields</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Still Glitchy</font><BR>2Feb23</td></tr>
<tr><td> - Rats find/harass Players in space</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>2Feb20</td></tr>
<tr><td> - Rat Wreck Creation</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>12Feb17</td></tr>
<tr><td> - Rat Loot</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>12Feb17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Science_and_Industry"> Science &amp; Industry (<font color=pink>Rewrite on hold</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:38%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:38%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>38.1</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Blueprints<BR></td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>11Jan25</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Players</h3></font></td></tr>
<tr><td> - Research</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>30Mar18</td></tr>
<tr><td> - Invention</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>6Mar15</td></tr>
<tr><td> - Reverse Engineering</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>6Mar15</td></tr>
<tr><td> - Ore Mining</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>30Mar18</td></tr>
<tr><td> - Gas Mining</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>2Feb20</td></tr>
<tr><td> - Refining and Reprocessing</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>30Mar18</td></tr>
<tr><td> - Manufacturing</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>30Mar18</td></tr>
<tr><td> - R&amp;D using Agents</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>30Nov15</td></tr>
<tr><td colspan=3><font color=white><h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Corporations</h3></font></td></tr>
<tr><td> - Research</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30May19</td></tr>
<tr><td> - Invention</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30May19</td></tr>
<tr><td> - Reverse Engineering</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30May19</td></tr>
<tr><td> - Refining and Reprocessing</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30May19</td></tr>
<tr><td> - Manufacturing</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>30May19</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Scan_and_Probing"> Scanning &amp; Probing (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:90%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:90%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>94.3</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - System Scanning using Ship Sensors (Without Probes)</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>21May16</td></tr>
<tr><td> - Directional Scanning (using Ship Sensors)</td><td class="working">80%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 80%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Still Glitchy</font><BR>2Feb23</td></tr>
<tr><td> - Launch Probes from ship</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar18</td></tr>
<tr><td> - Move Probes in Space</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar18</td></tr>
<tr><td> - Change Probe Range</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>6Mar18</td></tr>
<tr><td> - System Scanning with Probes  (See 'Scan Manager' Data Block below for details)</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>6Mar18</td></tr>
<tr><td> - Bookmark Scanned Result</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Found Errors</font><BR>2Feb23</td></tr>
<tr><td> - Warp to Scanned Result</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=orange>Found Errors</font><BR>2Feb23</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Fleet_System"> Fleet System (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:96%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:96%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>95.7</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Create Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Fleet management window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Set Fleet MOTD</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>1Dec17</td></tr>
<tr><td> - Fleet Warp</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>20Feb20</td></tr>
<tr><td> - View Fleet Composition</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Create/Delete Wings and Squads</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Name Wings and Squads</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Invite to Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Accept Fleet Invite</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Reject Fleet Invite</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Leave Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Kick Member</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Move fleet members</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Set commanders</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Set boosters</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Commanders provide bonuses to their members</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>1Dec17</td></tr>
<tr><td> - Boosters provide bonuses to their members</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>1Dec17</td></tr>
<tr><td> - Broadcast to fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Broadcast to system</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Broadcast to bubble</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Add Member to watch list</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>1Dec17</td></tr>
<tr><td> - Remove Member from watch list</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>1Dec17</td></tr>
<tr><td> - Fleet chat window</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Advertise Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - View Availible Fleets</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - Apply to Advertised Fleet</td><td class="working">100%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 100%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=green>COMPLETE</font><BR>1Dec17</td></tr>
<tr><td> - View Fleet Applications</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>1Dec17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Planetary_Interaction"> Planetary Interaction System (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:71%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:71%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>71.6</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Planet Search</td><td class="working">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>27Dec16</td></tr>
<tr><td> - Planet View</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Dec16</td></tr>
<tr><td> - Planet Resource Survey</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Dec16</td></tr>
<tr><td> - Install/Use Command Center </td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>
Recent SOV updates broke CC installation<BR>3Feb23</td></tr>
<tr><td> - Creating Extractor Heads</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Dec16</td></tr>
<tr><td> - Install/Use Silo</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Dec16</td></tr>
<tr><td> - Creating logistic routes (links) on planet</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Dec16</td></tr>
<tr><td> - Installing Schematics</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Dec16</td></tr>
<tr><td> - Factory Automation (run schematics)</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Dec16</td></tr>
<tr><td> - Install/Use Space Ports</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>27Dec16</td></tr>
<tr><td> - Planetary Customs Office</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>27Dec16</td></tr>
<tr><td> - Launching Items to Customs Office</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>27Dec16</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Corporation_Management"> Corporation Management (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:67%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:67%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>67.7</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Create a corporation</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>10Dec17</td></tr>
<tr><td> - Managing Members</td><td class="working">70%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 70%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>20Dec17</td></tr>
<tr><td>   - Apply to Join Corp</td><td class="broken">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28May19</td></tr>
<tr><td>   - Invite to Join Corp</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>20Dec17</td></tr>
<tr><td>   - Quit Corp</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>20Dec17</td></tr>
<tr><td>   - Kick from Corp</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Mostly Non-Working</font><BR><font color=olive>Testing</font><BR>20Dec17</td></tr>
<tr><td> - Managing Roles</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Managing Divisions</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Managing Recruitment</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td>   - Creating/Editing Advert</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Managing Funds</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Managing Voting</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28May19</td></tr>
<tr><td> - Viewing Roles</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Viewing Divisions</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Viewing Members</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>20Dec17</td></tr>
<tr><td> - Viewing Corp History</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>10Dec17</td></tr>
<tr><td> - Viewing Journal</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28May19</td></tr>
<tr><td> - Viewing Transactions</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Viewing Bills</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>10Dec17</td></tr>
<tr><td> - Viewing Assets</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28May19</td></tr>
<tr><td> - Renting Office</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Moving HeadQuarters</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Declaring War</td><td class="broken">20%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 20%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>Broken</font><BR><font color=magenta>Needs Work</font><BR>10Dec17</td></tr>
<tr><td> - Moving Items In and Out of corp hangars</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>20Dec17</td></tr>
<tr><td> - Market Deliveries and Returns in Station</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>20Dec17</td></tr>
<tr><td> - Impounded Items</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>20Dec17</td></tr>
<tr><td> - Creating Medals</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28May19</td></tr>
<tr><td> - Viewing Medals</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28May19</td></tr>
<tr><td> - Awarding Medals</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28May19</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Alliance_Management"> Alliance Management (<font color=yellow>needs review after adding James' SOV updates</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:15%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:15%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>15.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Creating Alliance</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>Begin Testing<BR>25May21</td></tr>
<tr><td> - Join Alliance</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>Begin Testing<BR>25May21</td></tr>
<tr><td> - Alliance Bulletins</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>Begin Testing<BR>25May21</td></tr>
<tr><td> - Alliance History (Corp Employment)</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>Begin Testing<BR>25May21</td></tr>
<tr><td> - Alliance Wars</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>Begin Testing<BR>25May21</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="POS_Operations"> POS - Player Owned Structures (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:61%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:61%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>61.4</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Deploying Structures</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28Dec17</td></tr>
<tr><td> - Scoop Structures to Cargo Bay</td><td class="working">40%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 40%; background:#00DB00">
</td><td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table>
<td><font color=yellow>Needs More Testing</font><BR>28Dec17</td></tr>
<tr><td> - Anchoring Structures</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28Dec17</td></tr>
<tr><td> - Onlining Structures</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28Dec17</td></tr>
<tr><td> - Managing Tower</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28Dec17</td></tr>
<tr><td> - Managing Structures Access</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>28Dec17</td></tr>
<tr><td> - Managing Tower Field Access</td><td class="broken">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>28Dec19</td></tr>
<tr><td> - Can Add/Remove Fuel and Strontium in Tower Holds</td><td class="working">95%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 95%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Final Testing</font><BR>28Dec17</td></tr>
<tr><td> - Reinforcing Tower</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>28Dec17</td></tr>
<tr><td> - Interaction with Structures</td><td class="working">50%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 50%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=violet>Somewhat Working</font><BR><font color=olive>Testing</font><BR>28Dec17</td></tr>
<tr><td> - Can store/retrieve ships from Ship Maintenance Arrays</td><td class="working">50%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 50%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=violet>Somewhat Working</font><BR><font color=olive>Testing</font><BR>28Dec17</td></tr>
<tr><td> - Can store/retrieve items from Corporate Hangar Arrays</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>28Dec17</td></tr>
<tr><td> - Can Add/Remove Charges for Offensive Batteries</td><td class="working">50%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 50%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=violet>Somewhat Working</font><BR><font color=olive>Testing</font><BR>28Dec17</td></tr>
<tr><td> - Using Structures</td><td class="working">50%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 50%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=violet>Somewhat Working</font><BR><font color=olive>Testing</font><BR>28Dec17</td></tr>
<tr><td> - Can Take Control of  Offensive Batteries</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>28Dec17</td></tr>
<tr><td> - Moon Mining</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>28Dec17</td></tr>
<tr><td> - Processing Moon Goo</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>28Dec17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Cosmic_Management"> Cosmic Management System (Details in Specific System Listing)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:47%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:47%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>46.9</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Anomaly Manager</td><td class="working">21%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 21%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>27Aug17</td></tr>
<tr><td> - Dungeon Manager</td><td class="broken">30%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 30%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>27Aug17</td></tr>
<tr><td> - Belt Manager</td><td class="working">79%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 79%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Aug17</td></tr>
<tr><td> - Spawn Manager</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Aug17</td></tr>
<tr><td> - Scan Manager</td><td class="working">78.7%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 78%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>27Aug17</td></tr>
<tr><td> - WormHole Manager</td><td class="broken">23%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 23%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>27Aug17</td></tr>
<tr><td> - Civilian Manager</td><td class="broken">6%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 6%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>27Aug17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Anomaly_Manager"> Anomaly Manager (<font color=yellow>needs review</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:21%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:21%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>20.6</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Gravametric Site Creation</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>12Feb17</td></tr>
<tr><td> - Gravametric Site Destruction</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>2Feb20</td></tr>
<tr><td> - Magnetometric Site Creation</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Magnetometric Site Destruction</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Radar Site Creation</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Radar Site Destruction</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Ladar Site Creation</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Ladar Site Destruction</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Dungeon_Manager"> Dungeon Manager (<font color=yellow>needs review</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:30%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:30%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>40.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Anomaly Dungeon Creation</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>12Feb17</td></tr>
<tr><td> - Anomaly Dungeon Destruction</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>2Feb20</td></tr>
<tr><td> - Mission Dungeon Creation</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>12Feb17</td></tr>
<tr><td> - Mission Dungeon Destruction</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>12Feb17</td></tr>
<tr><td> - Unrated Dungeon Creation</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>12Feb17</td></tr>
<tr><td> - Unrated Dungeon Destruction</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>2Feb20</td></tr>
<tr><td> - Unrated Escalation Creation</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>12Feb17</td></tr>
<tr><td> - Unrated Escalation Destruction</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>12Feb17</td></tr>
<tr><td> - DED Complex Creation</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>12Feb17</td></tr>
<tr><td> - DED Complex Destruction</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>12Feb17</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Belt_Manager"> Belt Manager (<font color=yellow>needs review</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:82%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:82%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>82.2</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Asteroid Belt Creation</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>2Feb20</td></tr>
<tr><td> - Asteroid Belt Destruction</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>2Feb20</td></tr>
<tr><td> - Asteroids Grow when idle</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>2Feb20</td></tr>
<tr><td> - Asteroids Disappear when mined out (individual roids)</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>2Feb20</td></tr>
<tr><td> - Asteroids Belts Respawn when empty</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=lime>Complete</font> - <font color=pink>PlayTesting</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Spawn_Manager"> Spawn Manager (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:85%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width: 85%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>85.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Dynamic spawning (Belt, Mission, Deadspace)</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>11Dec18</td></tr>
<tr><td> - Static spawning (Gate)</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1">
<tr><td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>11Dec18</td></tr>
<tr><td> - Spawn Destruction/Removal</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>11Dec18</td></tr>
<tr><td> - Spawns Warp into Belts</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>11Dec18</td></tr>
<tr><td> - Spawns Warp out of Belts</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>11Dec18</td></tr>
<tr><td> - Spawns guard Gates</td><td class="working">85%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 85%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=brown>Mostly Complete</font> - <font color=pink>PlayTesting</font><BR>11Dec18</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Scan_Manager"> Scan Manager (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:78%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:78%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>78.7</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Probe Acquiring Data based on Probe's Attributes and Character Skills</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>11Dec18</td></tr>
<tr><td> - Anomaly Data compilation based on Probe Results</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>11Dec18</td></tr>
<tr><td> - Reporting of Compiled Data</td><td class="working">90%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 90%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=pink>Final Testing</font><BR>11Dec18</td></tr>
<tr><td> - Probe Management</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>11Dec18</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="WormHole_Manager"> WormHole Manager (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:23%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:23%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>23.0</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - WormHole Creation</td><td class="working">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>12Feb17</td></tr>
<tr><td> - WormHole Tracking</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>12Feb17</td></tr>
<tr><td> - WormHole Destruction</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>12Feb17</td></tr>
<tr><td> - View WormHole Details</td><td class="working">75%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 75%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=teal>Working</font> - <font color=olive>Still Testing</font><BR>12Feb17</td></tr>
<tr><td> - Jumping thru WormHole</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
<h2> <span class="mw-headline" id="Civilian_Manager"> Civilian Manager (<font color=green>Current</font>)</span></h2>
<div id="Main_progress_bar_appearance_and_calculation" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 5px">
<table style="background: transparent; border:3px solid blue; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: #2222EE; width:6%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td></td></tr></table>
</td></tr></table></div></div><div id="Current_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 8px">
<table style="background: transparent; border:2px clear; width:100%; height:50px; -moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr><td>
<table style="background: transparent; width:8%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> &#160;<font color=red><b>5.8</b></font>&#160;&#160;&#160;&#160;</td></tr></table>
</td></tr></table></div></div><div id="goal_value_marker" style="position:relative; width:100%"><div style="position:absolute; width:100%; top: 11px">
<table style="background: transparent; border:2px clear; width:100%; height:50px;-moz-border-radius: 110px; -webkit-border-radius: 110px; border-radius: 110px; text-align:right"><tr>
<td> <font color=red><b>100</b></font>&#160;&#160;&#160;&#160;</td></tr></table></div></div><br /><p><br /><br /></p><p><br /></p>
<table class="statusTable" border="1" style="width: 100%; border-collapse: collapse; border-style: solid;border-width: 1px;border-color: #666;">
<tr><td style="width: 70&amp;;">Service/Action</td>
<td style="width: 4%;">Status</td>
<td style="width: 26%;">Comments</td></tr>
<tr><td> - Civilian Ship Creation</td><td class="broken">15%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 15%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=yellow>Preliminary Implementation</font><BR>12Feb17</td></tr>
<tr><td> - Civilian Ships "Undock" from station</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>12Feb17</td></tr>
<tr><td> - Civilian Ships "Dock" to station</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Civilian Ships Travel in System</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Civilian Ships Travel using Jump Gates</td><td class="broken">0%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 1%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=red>NOT IMPLEMENTED</font><BR>2Feb20</td></tr>
<tr><td> - Trader Joe traveling with Civilian Ships</td><td class="broken">10%
<table style="text-align: left; width: 150px; border: 1px solid black" cellspacing="1"><tr>
<td style="text-align: left; height: 12px; width: 10%; background:#00DB00"></td>
<td style="text-align: left; height: 15px; background:#ffffff"></td></tr></table></td>
<td><font color=orange>Skeleton Code Only</font><BR>2Feb20</td></tr>
</table>
<p><br style="clear:both;" /></p>
</table><BR><BR>


<!--  Modern browsers (since 2005) support 140(147) named colors
17 vga colors are aqua, black, blue, fuchsia, gray, green, lime, maroon, navy, olive, purple, red, silver, brown, white, and yellow
COLOR				PHASE				ESTIMATED COMPLETION
<green> 		COMPLETE						= 100%
<lime>			Final Testing					= 95
<lime/pink>		Complete - PlayTesting			= 90
<brown/pink>	Mostly Complete - PlayTesting	= 85
<brown/orange>	Mostly Complete - Still Glitchy	= 80
<brown/orange>	Mostly Complete - Still Buggy	= 80
<brown/olive>	Working - Testing				= 75
<brown/magenta>	Working - Needs Work			= 70
<orange/silver>	Can Exploit - Needs Update		= 60
<violet/olive>	Somewhat Working - Testing		= 50
<yellow>		Needs More Testing				= 40
<yellow/olive>	Mostly Non-Working - Testing	= 30
<orange>		Hacked							= 25
<red/magenta>	Broken - Needs Work				= 20
<yellow>		Preliminary Implementation		= 15
<orange>		Skeleton Code Only				= 10
<red>			NOT IMPLEMENTED					= 0


green
lime
brown
brown
violet
yellow
orange
red

pink
olive
magenta
silver

-->

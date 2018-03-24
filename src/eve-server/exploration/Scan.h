 /**
  * @name Scan.h
  *   Scanning methods for Alasiya EvE
  *
  * @Author:        Allan
  * @date:          7Dec15 (working)
  *
  */


#ifndef EVEMU_SCANING_SCAN_H_
#define EVEMU_SCANING_SCAN_H_

#include "packets/Scan.h"
#include "system/cosmicMgrs/ManagerDB.h"

struct SignalData {
    float distance;
    float certainty;
    float deviation;
    CosmicSignature sig;
    PyRep* probes;
    PyRep* probePos;
};


class PyRep;
class Client;
class ProbeSE;
class SystemManager;

class Scan {
public:
    Scan(Client* pClient);
    //Scan(Scan& scan);
    ~Scan()                                             { /* do nothing here */ }

    // called by client timer, which was set for probeStateChange
    void ProcessScan(bool useProbe=false);

    PyRep* ConeScan(Call_ConeScan args);
    void RequestScans(PyDict* dict);
    void ShipScanResult();
    void ProbeScanResult();

    void SystemScanStarted(uint16 duration);

    // probe methods
    void AddProbe(ProbeSE* pProbe);
    void RemoveProbe(ProbeSE* pProbe);

    bool GetProbeDataForSig(SignalData& data);
    // this will determine signal strength, deviation, and other aspects based on signal type and probe data
    void GetSignalData(SignalData& data, std::vector< ProbeSE* >& probeVec, GPoint& point);

protected:
    void ScanStart();
    void SurveyScan();

private:
    ManagerDB* m_db;
    Client* m_client;
    SystemManager* m_system;

    bool m_probeScan;

    std::map<uint32, ProbeSE*> m_probeMap;
};

#endif // EVEMU_SCANING_SCAN_H_



/** @note:  Scanning Eratta  .....
 *
 This post will try to serve as a guide to the scan probe* system EVE currently uses (for Apoc) and I will do my best to keep it updated with any changes and other relevant information.

 Scan probing in Revelations

 This guide will focus on ship probing, look here for a guide to exploration by Joerd Toastius. This guide assumes you posses at least basic control over how the Directional Scanner work.

 If you have any further questions about ship probing feel free to contact me ingame BUT please do not ask me about exploration, I don't do exploration and won't be able to answer your questions.

 To do scan probing we need a few things, namely a probe launcher, probes, a ship to mount it on and the skills to use them all.

 Skills


 Astrometrics - Adds one scan group per level. Is the primary skill that determines what probes you can use.
 For combat ship probing level 3 will do fine unless the target is in a deep safe for which level 5 is needed.

 Astrometric Pinpointing – Reduces maximum scan deviation by 10% per level.
 Means that you can use a weaker probe and still get a result that lands you in the same grid as the target.

 Astrometric Triangulation – 5% scan strength bonus per level of skill.
 Higher scan strength means easier to find the target and more accurate results.

 Signal Acquisition - 10% faster scanning with scan probes per level.
 Must have skill, will cut scan time in half at level 5. Sadly it’s rank 8.

 Other skills like Survey do NOT effect probing.

 Probe Launchers

 There are 4 different probe launchers. 2 classes with 2 variations in each.
 The Scan Probe Launcher I and faction version Sisters Scan Probe Launcher.
 The Recon Probe Launcher I and faction version Sisters Recon Probe Launcher.
 The Scan and Recon Probe Launcher I has similar fittings (220cpu, 1pg / 220cpu, 2pg). For the faction versions the Sisters Scan Probe Launcher has 25% shorter activation time while the Sisters Recon Probe Launcher only sports 10 cpu less fitting requirement.

 Despite its name the Recon Probe Launcher can be used on any ship, it's not exclusive to recon ships.

 For Exploration and Moon Survey


 Scan Probe Launcher I – 600 sec base cycle time (can be cut down to 109.35 sec). 10m3 capacity. 15 sec rate of fire.

 Sisters Probe Launcher I - 450 sec base cycle time (can be cut down to 82.01 sec). 10m3 capacity. 15 sec rate of fire.

 For Ship Probing


 Recon Probe Launcher I – 120 sec base cycle time (can be cut down to 21.87 sec). 1m3 capacity. 2.5 sec rate of fire.

 Sisters Recon Probe Launcher - Same stats as Recon Probe Launcher I except fitting.

 It’s not possible to fit multiple probe launchers on a ship, not even offline.

 Probes

 There are 3 different groups of probes. Ship probes, exploration probes and survey probes.
 Ship probes are the probes that fit into the Recon Probe Launcher. They include:


 Observator Deep Space Probe I - 1000 au range, 1.25 point sensor strength, 20.000 km max scan deviation, 4800 sec flight time.

 Sisters Observator Deep Space Probe I - 1000 au range, 1.4 points sensor strength, 20.000km max scan deviation, 4800 sec flight time.

 Ferret Scanner Probe I - 40 au range, 2.5 points sensor strength, 10.000 km max scan deviation. 2400 sec flight time.

 Spook Scanner Probe I - 20 au range, 5 points sensor strength, 5.000 km max scan deviation, 1200 sec flight time.

 Fathom Scanner Probe I - 10 au range, 10 points sensor strength, 2.500 km max scan deviation, 600 sec flight time.

 Snoop Scanner Probe I - 5 au range, 20 points sensor strength, 200 km max scan deviation, 300 sec flight time.

 Range - The max range of the probe, this is a 3d sphere and the probe will not be able to find anything outside this range. Reports say that Observator probe have unlimited range and not the 1000au listed.
 Sensor Strength - Higher sensor strength means larger chance to find the target and more accurate results.
 Max Scan Deviation - The maximum distance from the target any scans with this probe will give.
 Flight Time – The amount of time the probe stays in space. You need to finish any scan before the flight time runs out or it will fail.

 All the other probes are uninteresting for ship scanning. While the exploration probes have very high sensor strength they also have very low range making them unsuited for normal ship probing.

 Ships

 The ship of choice for probing is the Covert Ops frigates. There are 2 reasons for this, one is the built in bonus of 10% reduction to scan time per level (level 5 cuts the scan time in half) and the other is the ability to warp cloaked. If you can’t get a 0 m accuracy result you will need to warp in cloaked and approach manually.

 Other ships that are useful for probing is the cloaking force recon ships for the warp cloaked ability and possibly the t1 astrometrics frigates as they have a 5% reduction to scan time per level. But in worst case any ship can be used.

 Basic probing
*************************
 So your target is sitting in afk in a safe spot and you want to find him? This is how it’s done.

 First warp around a bit and see if you can find him on the directional scanner. If you can see him; get to the closest object and drop the needed probe. Use the range option on your scanner to determine which probe is needed. 5 au = 750.000.000 km, 10 au = 1500.000.000 km. So if you can get within 750.000.000 km use the 5au probe etc. Max range of the directional scanner is 14.35 au.

 If you can’t find him on the scanner you need to use longer range probes, probably Observator Deep Space Probes or possibly 40 au Ferret probes.

 After you launch the probe open your scanner, select the System Scanner tab, select the probe and select the "Ships" group (you might as well include as many groups as your astrometrics skill allow, there is no penalty in using several groups).

 Click "Analyze" at the bottom of the window. A timer will appear counting down. If you want you can cloak now. You can switch to the Directional Scan tab and use that without breaking the probe and you can even close the scan window and it will still work. You can also warp around but doing so can bug the interface a bit. Just wait for the timer to finish and you will get the correct result.

 When the timer reaches 0 you will get a list with results. If your target is not in this list don’t fret it. The probing system is now chance base and you might need to scan several times (20+ if you are looking for a very small ship with an observator probe). But first recheck your scanner to see that he is in range of the probe you choose. If he is in range just click "New Scan" and hit analyze again. Repeat until the target is found.

 You can also see the results on the system map as colored dots. The color indicate the Signal strength.
 0-0.4 = red
 0.4-0.8 = yellow
 0.8+ = green
 You can warp to the results by right click on them and choose warp to.

 In the result list there are 4 columns. The first is ship type, second is signal strength (more about that in the advanced guide), third is the range from your current position and forth is Accuracy. Accuracy is the range from the spot the probe provides to the target.

 Accuracy is determined by several factors where the most important is the probe type and signal strength (see advanced section for formulas).
 Longer range probes give larger max deviation from the target. There is also some randomness involved here so a new scan might give a more accurate result (or worse).
 If you can get the Signal Strength above 1.0 with 1 probe you will always get 0 m accuracy and can warp in right on top of the target.
 As long as you are using ship probes you should never get an accuracy result above 20.000 km.

 If the accuracy is not good enough to get inside the same grid as the target you might need to launch and scan with a shorter range probe.
 Warp to the result by right clicking on it and choose warp to. Click new scan and right click on the probe you used and choose "destroy probe".
 The reason for this is the fact that you can not launch a probe within the scan radius of another probe. Now launch a 5au (snoop) probe and scan with that.
 Worst case scenario with that probe will land you just 200km away, most likely a lot closer.

 Advanced Probing
*****************************
 Signal Strength
 Signal Strength decides how large the chance is that the target will show up on a given scan and also effect accuracy.
 A Signal strength of 0.5 means 50% chance, 1.0 or more give 100% chance etc.

 Signal strength is a factor of the sensor strength of the probe, the signal size of the target, the range from the probe to the target and any skills you might have.
 Sensor strength is listed in the probe attributes.
 Target Signal Size = Target Signature Radius / Target Sensor Strength

 Large target = easier to find, target with high sensor strength = harder to find.
 These values can be modified, a target with several shield extenders will have larger signature radius and be easier to find while a target with ECCM will
 have higher sensor strength and will be harder find.

 The following formula (discovered by Daron) give the range multiplier.

 Range Multiplier = e^-((Target Range / Max Range)^2)
 Target Range is the range from the probe to the target, Max Range is the Scan Range listed for that probe type.

 This formula will return a result between 1 (at 0km) and 0.3679 (at very close to max range).

 The full formula to calculate Signal Strength is:
 Signal Strength = (Probe Sensor Strength * (1 + Level of Astrometric Triangulation * 0.05) / 100) * (e^-((Target Range / Max Range)^2)) * (Target Signature Radius / Target Sensor Strength)

 A math example: We are using a Ferret 40 au probe to try to locate a Scorpion 35 au away. We have Astrometric Triangulation level 3.
 Signal Strength = (2.5 * (1 + 3*0.05) / 100) * (e^-((35/40)^2)) * (480 / 24) = 0.267 or 26.7% chance it will show up on our scan.

 Accuracy
 Accuracy is a factor of the max scan deviation, the signal strength of the scan, any skills you might have and a random number.

 If the signal strength of the probe is 1.0 or more the accuracy will always be 0m (unless you are using multiple probes).

 The formula to calculate Maximum effective Scan Deviation is not know at this time, the following formula will provide an estimate that works for most cases:
 Maximum effective Scan Deviation = Maximum Scan Deviation * ((0.6 * (Signal Strength ^ 2)) – (1.6 * Signal Strength) + 1) * (1 - Level of Astrometric Pinpointing * 0.1)

 The accuracy of the scan is then a linear random range between 0km and the Max effective Scan deviation.
 Linear meaning it’s just as likely to return 0 as it is to return max eff scan dev or anything between.

 Math example: We will use the scorp from the last example, we also have astrometric pinpointing level 3.
 Max effective Scan Deviation = 10000 * ((0.6 * (0.267 ^ 2)) - (1.6 * 0.267) + 1) * (1 - 3 * 0.1) = 4305km.
 So each successful scan will give a random accuracy between 0km and 4305km. This means there is a 11.6% chance you will get a result in the same grid as the target.

 Directional Scanner
 It’s possible to see probes on the directional scanner by setting it to not use overview settings.
 To counter act this we have the ability to destroy the probes at will by right clicking on it in the system scanner tab and choose destroy probe,
 you can even do this while cloaked or in warp. Destroying a probe will not destroy the results so you can go back and look at them by clicking view results.
 You should destroy your probes as soon as you got the results you want from them, especially short range probes.
 Done right it should only show up on the scanner of the target for around 30-40 sec, short enough time for him to miss it.

 Deadspace Areas
 Deadspace areas like most mission and most exploration sites acts as a dampener on a ships Signal Size making it much harder to find targets in such sites.
 The exact amount of dampening is unknown but in the area of 100 times.

 Cloaked ships
 At the time of writing it is NOT possible to probe for cloaked ships.
 Latest information on the subject says it won't be possible until the planned full rework of the scan system is put in place.

 Scan Groups
 The following Scan Groups can be chosen when you start a scan, for each level of astrometrics you can scan for more group at the same time.

 Drone and Probe- As the name suggests, drones and probes (not interdictor probes, just scan probes).
 Cosmic Anomaly - NPC Combat sites
 Ship - All player ships, no npcs.
 Cosmic Signature - Exploration content.
 Structure - POS Structures.

 There is no penalty for scanning for multiple groups, if you have astrometrics level 5 go ahead and select them all.

 Known Bugs
 Attributes window show sensor strength truncated.
 This is most apparent on Observator and Ferret probes as it is shown to have 1 and 2 points strength but in reality they have 1.25 and 2.5 points.

 Warping while running a scan can sometimes bug the interface so it looks like the scan failed. Just wait for the scan to finish and you will still get the correct result.




Space Wanderer
Posted - 2009.03.11 00:16:00 - [2]

Here is a close approximation of the formula employed by the game to evaluate the combined scan strength of four (or less) probes.
This is what could be derived by trial and error on the last patch on Sisi. I suppose it should still be valid for Tranquillity.

The formula proceeds in separate steps:
1) Evaluate the scan strength of each probe.
2) Evaluate the angles of each couple of probes with site.
3) Choose the probes (four or less) that will be involved in evaluating the sig strength.
4) Evaluate the total signal strength.

1) Evaluate the scan strength of each probe.
    This proceeds very similar to the formula in the previous system
        sig-str% = Size * probe-str * distance-modifier / 2
    where
        probe str = prob-base-str*bonusmodifiers(skills,ship, equip,etc)/range modifier (1, 2, 4, 8... depending on the range the probe has been set to),
        distance-modifier = e^-((Target Range / Max Range)^2).

2) Evaluate the angles of each couple of probes with site.
    For each couple of probes the game determine the angle P1--target--P2.
    First, the angles are counted only up to 90 degrees.
    Any angle larger than 90 degrees is actually counted as 90 degrees.
    Less than 90 degrees will lead to sig str reduction, while more won't have any increased beneficial effect.

3) Choose the probes (four or less) that will be involved in evaluating the sig strength.
    By the words of greyscale, only the "best" four probes are employed to evaluate the signal strength.
    How would the server choose which of the potentially eight probes are the "four" best probes?
    In my observation this is how it goes:
    a) First the game chooses which probes are the "better" to use.
        This is probably done by creating a ranking of probes.
        This ranking is certainly not based either on the 1 probe sig str, nor on the best angles.
        In my observations it seems to be based on a combination of both, according to the following formula:
            rank-value = (angle1 + angle2 + angle3+....)*sig-str.
        The highest ranked probes are chosen.
        Note that this formula does not guarantee to always choose the probes that give you the best sig str.
        I have been able to force the game to choose suboptimal probes.
    b) Once a ranking has been established among probes, the game choose how many probes are to be employed.
        This is probably done in the following way...
        the amount of all angles between probes is summed.
        If it is larger than 270 (6 angles at 45?), four probes are employed.
        If it is more than about 180 (three 45/50 angles), three probes are employed.
        If it is less than that two probes are employed.
        If the sum of angles is less than 135 degree the usual X separate 1-probe signals are given.
        Note: this special case formula is ONLY valid in case we have four probes or more available.
        You can read formula formula generalized to less probes cases in the post below.

4) Evaluate the total signal strength.

Once the probes have been chosen, the final formula is:

combined-sig-str% = (average of 1-probe sig-str)*2*(angle1 + angle2 + angle3 + angle4)/360.

Note that in my observations only the 4 highest angles between probes are used in the calculations.
Note also that the "*2" factor rebalances the "/2" factor introduced in the 1-probe signal strength.



Generalized formula to evaluate how many probes to use

The formula seem to work in this way:
first, the server counts the number of angles between active probes.
    If there are 4 or more active probes there are 6 angles, for 3 probes there are 3 angles, and for 2 probes 1 angle.
Then those angles are summed.
    If the result of the sum is higher than half the maximum (90*Nangles/2) all possible probes are employed.
    If the result is lower than that, but higher the 1/3 of the maximum (90*Nangles/3) one probe less is employed.
    If the result is lower than that, but higher the 1/4 of the maximum (90*Nangles/4) two probe less are employed.
    If the result is lower than that, or you just can't subtract anymore probes, we fall into the separate signals case.

This translates in the following special cases:
4 probes or more: more than 270, 4 probes; more than 180, 3 probes; more than 135, 2 probes; less than 135, separate signals.
3 probes: more than 135, 3 probes; more than 90, 2 probes; less than 90, separate signals.
2 probes: more than 45, 2 probes; less than 45, separate signals.


Some Comments:
1) Tetrahedron formation around the target to be found is optimal.
    While it is not the ONLY optimal formation, it is the one that gives more margin for mistakes.
    A planar cross formation is as optimal as tetrahedron, but even a small mistake in probe placement or deviation would drastically reduce the signature strength.
    Tetrahedron is a bit more resistant to unforeseen mistakes.
2) Keeping more than one probe in the same position of another not necessarily will generate an echo.
    It might contribute to the scan (although with a low contribution) if you don't exceed with it.
3) The computer's choice about the "best possible probes" is not necessarily correct, although reasonably decent.
4) The decay law for angles modifiers is a linear law.
    Given the approach they followed I would have favored a sinusoidal decay...Would have made more sense.




General tips for placing probes around a (deviating) target and scanning in general:

Make sure you surround the target evenly.
    Usually your target will be the red of yellow dot as a result form an earlier scan.
    As mentioned by Space Wanderer its best to create a tetrahedron around the target using 4 probes.

Do not place your probes too close to a red or yellow dot. The dot is an estimated location of your target. Putting your probes too close can really screw up the angles which hurts your result.

If you are in wormspace and you lost your wormhole back use (deep space) probes and set its range so that combined it will give you 25% strength for signature size 10 (see above excel file). This will help you find another WH exit much faster than scanning each cosmic signature down in the system.

Do not use too many probes on a single target: they don't add more strength. Normally 4 works best. You can have more probes in space (sometimes its convenient to keep several long range probes out because there is no history kept of found signatures) but make sure you only scan with four of them (so deactive the rest).

When entering a system its handy to use a single deep space probe at 256 AU to have a quick looks at whats in a system. Using the excel file above you can get a sense of what kind of signatures are in a system. High target strengths mean easy stuff, low target strengths are the hard ones. Once you've done that you can use a shorter range probe setup (eg 4-8 AU) to get 25% target strengths on signatures to see if they're radar/ladar/grav/mag/unknown.

If for some reason you get split results or a unexpectedly bad signature strength (or "weird" results) it can be a good idea to check the result of each pair of probes (so deactivating the rest): if the result of a pair of probes is split or the strength is really low compared to other pairs then it probably means their angle to the target is too tight so you know you have to reposition those probes.


*/
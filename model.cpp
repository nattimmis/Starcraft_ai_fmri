#include <pl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include "enums_name_tables.h"
#include "x_values.h"
#include "replays.h"
using namespace std;

#define DEBUG_OUTPUT 1

/// TODO: use an iterator in values[] so that we directly
/// put a std::set instead of std::vector for terran_X/protoss_X/zerg_X
/// TODO: replace set by unordered_set (TR1 or boost) in a lot of places

///std::vector<std::set<Terran_Buildings> > terran_X;
std::vector<std::set<Protoss_Buildings> > protoss_X;
///std::vector<std::set<Zerg_Buildings> > zerg_X;

/** 
 * Test if the given X value (X plSymbol in plValues X_Obs_conj)
 * is compatible with what obervations have been seen
 * (observed plSymbol(s) in plValues X_Obs_conj)
 * {X ^ observed} covers all observed if X is possible
 * so X is impossible if {observed \ {X ^ observed}} != {}
 */
void test_X_possible(plValues& lambda, const plValues& X_Obs_conj)
{
    set<Protoss_Buildings> setX = protoss_X[X_Obs_conj[0]];
    set<Protoss_Buildings> setObs;
    set<Protoss_Buildings> intersect;
    //for (plValues::const_iterator it = X_Obs_conj.begin(); ...)
    for (unsigned int i = 1; i <= NB_PROTOSS_BUILDINGS; ++i)
    {
        if (X_Obs_conj[i])
        {
            setObs.insert(static_cast<Protoss_Buildings>(i-1));
            if (setX.count(static_cast<Protoss_Buildings>(i-1)))
                intersect.insert(static_cast<Protoss_Buildings>(i-1));
        }
    }

    vector<Protoss_Buildings> difference(setObs.size());
    vector<Protoss_Buildings>::iterator it = 
        set_difference(setObs.begin(), setObs.end(), 
                intersect.begin(), intersect.end(), 
                difference.begin());
    if (difference.begin() == it)
    {
#if DEBUG_OUTPUT > 3
        cout << "DEBUG: test_X_possible TRUE" << endl;
#endif
        lambda[0] = 1; // true
    }
    else
    {
#if DEBUG_OUTPUT > 3
        cout << "DEBUG: test_X_possible FALSE" << endl;
#endif
        lambda[0] = 0; // false
    }
}

int main(int argc, const char *argv[])
{
    std::vector<std::string> terran_openings;
    /*terran_openings.push_back("fast_drop"); // facto-starport-control tower
    terran_openings.push_back("full_metal"); // facto x2 + machine shop
    terran_openings.push_back("MM"); // raxes-academy
    terran_openings.push_back("fast_expand"); // CC first
    terran_openings.push_back("1rax_FE"); // rax-CC
    terran_openings.push_back("mech_timing_push"); // facto-armory-facto
    terran_openings.push_back("fast_air"); // starport x2
    terran_openings.push_back("BBS"); // rax-rax-supply
    terran_openings.push_back("unkown");*/
    terran_openings.push_back("Bio");
    terran_openings.push_back("TwoFactory");
    terran_openings.push_back("VultureHarass");
    terran_openings.push_back("SiegeExpand");
    terran_openings.push_back("Standard");
    terran_openings.push_back("FastDropship");
    terran_openings.push_back("Unknown");

    std::vector<std::string> protoss_openings;
    /*protoss_openings.push_back("fast_legs"); // core-citadel-gates-legs
    protoss_openings.push_back("fast_DT"); // citadel-archives-DT
    protoss_openings.push_back("fast_air"); // gate-core-stargates
    protoss_openings.push_back("fast_expand"); // nexus first
    protoss_openings.push_back("reaver"); // robot-support bay
    protoss_openings.push_back("standard"); // 2 gates-core-robo-observatory
    protoss_openings.push_back("goons"); // gate-core-gates-range (NonY)
    protoss_openings.push_back("proxy_gates"); // pylon-gates @enemy
    protoss_openings.push_back("photon_rush"); // forge-pylon @enemy
    protoss_openings.push_back("unknown");*/
    protoss_openings.push_back("FastLegs");
    protoss_openings.push_back("FastDT");
    protoss_openings.push_back("FastObs");
    protoss_openings.push_back("ReaverDrop");
    protoss_openings.push_back("Carrier");
    protoss_openings.push_back("FastExpand");
    protoss_openings.push_back("Unknown");


    std::vector<std::string> zerg_openings;
    /*zerg_openings.push_back("fast_pool"); // 4-8 pools
    zerg_openings.push_back("lings"); // early pool + no peons @ extractor
    zerg_openings.push_back("fast_mutas"); // early extractor-lair-spire
    zerg_openings.push_back("mass_hydras"); // expand-hydra den
    zerg_openings.push_back("mutas_into_hydras"); // expand-lair-spire-hatch-hydra
    zerg_openings.push_back("fast_lurkers"); // early gaz-lair-hydra den
    zerg_openings.push_back("unknown");*/
    zerg_openings.push_back("TwoHatchMuta");
    zerg_openings.push_back("ThreeHatchMuta");
    zerg_openings.push_back("HydraRush");
    zerg_openings.push_back("Standard");
    zerg_openings.push_back("HydraMass");
    zerg_openings.push_back("Lurker");
    zerg_openings.push_back("Unknown");

    ifstream fin("PvP.txt"); /// match up: Enemy vs Us
    protoss_X = get_X_values<Protoss_Buildings>(fin); /// Enemy race
    /// For instance ZvT will get Zerg buildings when Zerg is against Terran

    /**********************************************************************
      VARIABLES SPECIFICATION
     **********************************************************************/
    plSymbol X("X", plIntegerType(0, protoss_X.size()));
    std::vector<plSymbol> observed;
    plSymbol lambda("lambda", PL_BINARY_TYPE);
    for (unsigned int i = 0; i < NB_PROTOSS_BUILDINGS; i++)
    {
        // what has been observed
        observed.push_back(plSymbol(protoss_buildings_name[i], PL_BINARY_TYPE));
    }
    //plSymbol OpeningTerran("OpeningTerran", VALUES);
    plSymbol OpeningProtoss("OpeningProtoss", plLabelType(protoss_openings));
    //plSymbol OpeningZerg("OpeningZerg", VALUES);

    plSymbol Time("Time", plIntegerType(1,1080)); // 18 minutes

    /**********************************************************************
      PARAMETRIC FORM SPECIFICATION
     **********************************************************************/
    // Specification of P(OpeningProtoss)
    std::vector<plProbValue> tableOpeningProtoss;
    for (unsigned int i = 0; i < protoss_openings.size(); i++) 
        tableOpeningProtoss.push_back(1.0); // TOLEARN
    plProbTable P_OpeningProtoss(OpeningProtoss, tableOpeningProtoss, false);

    // Specification of P(X) (possible tech trees)
    std::vector<plProbValue> tableX;
    for (unsigned int i = 0; i < protoss_X.size(); i++)
        tableX.push_back(1.0); // TOLEARN
    plProbTable P_X(X, tableX, false);

    // Specification of P(O_1..NB_PROTOSS_BUILDINGS)
    plComputableObjectList listObs;
    std::vector<plProbTable> P_Observed;
    plProbValue tmp_table[] = {0.5, 0.5}; // TOLEARN (per observation)
    for (unsigned int i = 0; i < NB_PROTOSS_BUILDINGS; i++)
    {
        P_Observed.push_back(plProbTable(observed[i], tmp_table, true));
        listObs *= plProbTable(observed[i], tmp_table, true);
    }

    // Specification of P(lambda | X, O_1..NB_PROTOSS_BUILDINGS)
    plVariablesConjunction ObsConj;
    for (std::vector<plSymbol>::const_iterator it = observed.begin();
            it != observed.end(); ++it)
        ObsConj ^= (*it);
    plVariablesConjunction X_Obs_conj = X^ObsConj;
    plExternalFunction coherence(lambda, X_Obs_conj, test_X_possible);
    plFunctionalDirac P_lambda(lambda, X_Obs_conj , coherence);
    
    // Specification of P(T | X, OpeningProtoss)
    plCndLearnObject<plLearnBellShape> timeLearner(Time, X^OpeningProtoss);
    plValues vals(timeLearner.get_variables());
    string input;
    ifstream inputfile_learn(argv[1]);
    istream& inputstream = argc > 2 ? inputfile_learn : cin;
    if (argc > 2)
        cout << ">>>> Learning from: " << argv[1] << endl;
    map<unsigned int, int> count_X_examples;
    map<unsigned int, plValues> one_value_per_X;
    while (getline(inputstream, input))
    {
        if (input.empty())
            break;
        string tmpOpening = pruneOpeningVal(input);
        if (tmpOpening != "")
        {
            multimap<unsigned int, Building> tmpBuildings;
            getBuildings(input, tmpBuildings);
            tmpBuildings.erase(0); // key == 0 i.e. buildings not constructed
            std::set<Protoss_Buildings> tmpSet;
            for (map<unsigned int, Building>::const_iterator it 
                    = tmpBuildings.begin(); 
                    it != tmpBuildings.end(); ++it)
            {
                tmpSet.insert(static_cast<Protoss_Buildings>(
                            it->second.getEnumValue()));
                vals[OpeningProtoss] = tmpOpening;
#if DEBUG_OUTPUT > 1
                std::cout << "Opening: " << tmpOpening << std::endl;
#endif
                unsigned int tmp_ind = get_X_indice(tmpSet, protoss_X);
                vals[X] = tmp_ind;
#if DEBUG_OUTPUT > 1
                std::cout << "X ind: " << get_X_indice(tmpSet, protoss_X) 
                    << std::endl;
#endif
                vals[Time] = it->first;
#if DEBUG_OUTPUT > 1
                std::cout << "Time: " << it->first << std::endl;
#endif
                if (count_X_examples.count(tmp_ind))
                    count_X_examples[tmp_ind] = count_X_examples[tmp_ind] + 1;
                else
                {
                    count_X_examples.insert(make_pair<unsigned int, int>(
                                tmp_ind, 0));
                    one_value_per_X.insert(make_pair<unsigned int, plValues>(
                                tmp_ind, vals));
                }
                if (!timeLearner.add_point(vals))
                    cout << "ERROR: point not added" << endl;
                vals.reset();
            }
        }
    }
    for (map<unsigned int, int>::const_iterator it = count_X_examples.begin();
            it != count_X_examples.end(); ++it)
    {
        if (it->second == 0)
        {
            cout << "PROBLEM: We never encountered X: ";
            for (set<Protoss_Buildings>::const_iterator ibn
                    = protoss_X[it->first].begin();
                    ibn != protoss_X[it->first].end(); ++ibn)
            {
                cout << protoss_buildings_name[*ibn] << ", ";
            }
            cout << endl;
        }
        else if (it->second == 1)
        {
            cout << "(POSSIBLE)PROBLEM: We encountered X only one time: ";
            for (set<Protoss_Buildings>::const_iterator ibn
                    = protoss_X[it->first].begin();
                    ibn != protoss_X[it->first].end(); ++ibn)
            {
                cout << protoss_buildings_name[*ibn] << ", ";
            }
            cout << endl;
            // put another (slightly different) value
            one_value_per_X[it->first][Time] = 
                one_value_per_X[it->first][Time] + 2; /// totally arbitrary 2
            timeLearner.add_point(one_value_per_X[it->first]);
        }
    }
#if DEBUG_OUTPUT > 2
    cout << timeLearner.get_computable_object() << endl;
#endif

    /**********************************************************************
      DECOMPOSITION
     **********************************************************************/
    plVariablesConjunction knownConj = ObsConj^lambda^Time;
    plJointDistribution jd(X^OpeningProtoss^knownConj,
            P_X*P_OpeningProtoss*listObs*P_lambda
            *timeLearner.get_computable_object()); // <=> P_Time);
    jd.draw_graph("jd.fig");
#if DEBUG_OUTPUT > 0
    cout << "Joint distribution built." << endl;
#endif

    /**********************************************************************
      PROGRAM QUESTION
     **********************************************************************/
    plCndDistribution Cnd_P_Opening_knowing_rest;
    jd.ask(Cnd_P_Opening_knowing_rest, OpeningProtoss, knownConj);
#if DEBUG_OUTPUT > 0
    cout << jd.ask(OpeningProtoss, knownConj) << endl;
#endif

    if (argc < 2)
        return 0;
    ifstream inputfile_test(argv[2]);
    cout << endl;
    cout << ">>>> Testing from: " << argv[2] << endl;
    unsigned int noreplay = 0;
    while (getline(inputfile_test, input))
    {
        plValues evidence(knownConj);
        evidence[lambda] = 1;
        if (input.empty())
            break;
        string tmpOpening = pruneOpeningVal(input);
        if (tmpOpening != "")
        {
            multimap<unsigned int, Building> tmpBuildings;
            getBuildings(input, tmpBuildings);
            tmpBuildings.erase(0); // key == 0 i.e. buildings not constructed
#if DEBUG_OUTPUT == 1
            cout << "************** Replay number: "
                << noreplay << " **************" << endl;
            ++noreplay;
#endif
            evidence[observed[0]] = 1; // the first Nexus/CC/Hatch exists
            // we assume we didn't see any buildings
            for (unsigned int i = 1; i < NB_PROTOSS_BUILDINGS; ++i)
                evidence[observed[i]] = 0;
            
            // we assume we see the buildings as soon as they get constructed
            for (map<unsigned int, Building>::const_iterator it 
                    = tmpBuildings.begin(); 
                    it != tmpBuildings.end(); ++it)
            {
                evidence[observed[it->second.getEnumValue()]] = 1;
                evidence[Time] = it->first;
                
#if DEBUG_OUTPUT > 1
                cout << "====== evidence ======" << endl;
                cout << evidence << endl;
#endif

                plDistribution PP_Opening;
                Cnd_P_Opening_knowing_rest.instantiate(PP_Opening, evidence);
#if DEBUG_OUTPUT > 1
                cout << "====== P(Opening | rest).instantiate ======" << endl;
                cout << Cnd_P_Opening_knowing_rest << endl;
                cout << PP_Opening.get_left_variables() << endl;
                cout << PP_Opening.get_right_variables() << endl;
#endif
                plDistribution T_P_Opening;
                PP_Opening.compile(T_P_Opening);
#if DEBUG_OUTPUT >= 1
                cout << "====== P(Opening | evidence) ======" << endl;
                cout << T_P_Opening << endl;
#endif
            }
        }
    }


    /*plSerializer my_serializer();
    my_serializer.add_object("P_CB", P_CB);
    my_serializer.save("test.xml");*/

    // On Windows (Visual C++, MinGW) only.
#if defined(WIN32) || defined(_WIN32)
    cout << "Press any key to terminate..." << endl;
    getchar();
#endif
    /* */

    return 0;
}

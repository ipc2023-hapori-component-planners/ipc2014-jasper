/*
 * general_two_level_best_first_search.cc
 *
 *  Created on: Jul 4, 2012
 *      Author: fxie2
 */

#include "general_two_level_macro_rw_eager_best_first_search.h"

#include "globals.h"
#include "heuristic.h"
#include "open_list_parser.h"
#include "option_parser.h"
#include "successor_generator.h"
#include "g_evaluator.h"
#include "sum_evaluator.h"
#include "open_lists/tiebreaking_open_list.h"

#include <cassert>
#include <cstdlib>
#include <set>
using namespace std;

GeneralTwoLevelMacroRwEagerBestFirstSearch::GeneralTwoLevelMacroRwEagerBestFirstSearch(
        const SearchEngineOptions &options, OpenList<state_var_t *> *open,
        OpenList<state_var_t *> *local_open, bool reopen_closed,
        bool pathmax_correction, bool use_multi_path_dependence,
        ScalarEvaluator *f_eval) :
    GeneralTwoLevelEagerBestFirstSearch(options, open, local_open, reopen_closed,
            pathmax_correction, use_multi_path_dependence, f_eval), m_walker(g_rand_gen), m_current_state(*g_initial_state)
{
    m_switchCounter = 0;
    m_nuLocalTry = 0;
}


GeneralTwoLevelMacroRwEagerBestFirstSearch::~GeneralTwoLevelMacroRwEagerBestFirstSearch()
{
}

int GeneralTwoLevelMacroRwEagerBestFirstSearch::step()
{
    const int LOCAL_TRY_MAX = 10;
    const int LOCAL_SIZE = 1000;
    if (m_switchCounter++ < LOCAL_SIZE)
    {
        if (!WAIT_LOCAL_SEARCH)
        {
            return super::super::step();
        }
        else
        {
            //store current best heuristic
            m_preBestHeuristicValue.clear();
            const vector<int> &best_values =
                    search_progress.get_best_heuristic_values();
            for (size_t i = 0; i < best_values.size(); ++i)
            {
                //cout << "PreBestValue-" << i << ": " << best_values[i] << endl;
                m_preBestHeuristicValue.push_back(best_values[i]);
            }

            int tmp_re = super::super::step();

            if (check_improvement())
            {
                m_nuLocalTry = 0;
                m_switchCounter = 0;
            }

            return tmp_re;
        }
    }

    m_switchCounter = 0;//reset switchCounter;
    if (m_nuLocalTry < LOCAL_TRY_MAX)
    {
        bool improved = false;
        //  cout << "start step-" << i << endl;
        const int result = local_search(100, improved);
        if (result == SOLVED)
        {
            return SOLVED;
        }
        if (result == FAILED)
        {
            //    cout << "Failed" << endl;
        }
        if (improved)
        {
            m_nuLocalTry = 0;
            cout << "improved by local search!" << endl;
            return IN_PROGRESS;
        }

        m_nuLocalTry ++;
    } else {
    }

    return IN_PROGRESS;
}


int GeneralTwoLevelMacroRwEagerBestFirstSearch::local_search(unsigned int size, bool &improved)
{
    int current_best_h_value = -1;
    pair<SearchNode, bool> n = fetch_next_node();
        if (!n.second) {
            return FAILED;
        }
    SearchNode node = n.first;
    State s = node.get_state();
    m_current_state = s;

    if (heuristics.size() >= 2)
    {
        m_walker.prepare_for_walks(m_mrwParam, mrw_heuristics[0]);
        current_best_h_value
                = (search_progress.get_best_heuristic_values())[1];
    }
    else
    {
        m_walker.prepare_for_walks(m_mrwParam, heuristics[0]);
        current_best_h_value
                = (search_progress.get_best_heuristic_values())[0];
    }
    m_mrwParam.length_walk = initial_walk_len;
    int last_effective_walk = 0;
    for (int i = 0; i < size; ++i)
    {
        g_mw_total_count++;
        m_walker.random_walk(s, m_mrwParam.length_walk,
                m_mrwParam, -1, false);
        const WalkInfo &walkInfo = m_walker.get_info();
        if (current_best_h_value == -1 || walkInfo.value < current_best_h_value)
        {
            std::cout << "cur_h: " << current_best_h_value << ", rw_h: "
                    << walkInfo.value << endl;
            const bool EXPAND_PATH = true;
            if (EXPAND_PATH)
            {
                std::cout << "start adding path~~" << endl;
                expand_the_path(walkInfo.path);
                improved = true;
                std::cout << "finish adding path~~" << endl;
                return IN_PROGRESS;
            }
            else
            {
                MacroOperator *mo = new MacroOperator(walkInfo.path);
                g_macro_operators.push_back(mo);

                State succ_state(s, *mo);
                SearchNode succ_node = search_space.get_node(succ_state);
                heuristics[0]->evaluate(succ_state);

                open_list->evaluate(node.get_g() + get_adjusted_cost(*mo),
                        true);
                int succ_h = heuristics[0]->get_heuristic();
                succ_node.open(succ_h, node, mo);
                open_list->insert(succ_node.get_state_buffer());
                if (search_progress.check_h_progress(succ_node.get_g()))
                {
                    reward_progress();
                }
                improved = true;
                return IN_PROGRESS;
            }
        }


        // Increase the length of the walk if current_min has not be
        // decreased in n last walks.
        if (m_mrwParam.deepening) {
            int n = int(size * m_mrwParam.extending_period);
            if (i - last_effective_walk > n) {
                m_mrwParam.length_walk = m_mrwParam.length_walk * m_mrwParam.extending_rate;
                last_effective_walk = i;
            }
        }
    }

    return IN_PROGRESS;
}


int GeneralTwoLevelMacroRwEagerBestFirstSearch::local_step()
{
    pair<SearchNode, bool> n = fetch_local_next_node();
    if (!n.second) {
        return FAILED;
    }
    SearchNode node = n.first;

    State s = node.get_state();
    if (check_goal_and_set_plan(s))
        return SOLVED;

    vector<const Operator *> applicable_ops;
    set<const Operator *> preferred_ops;

    g_successor_generator->generate_applicable_ops(s, applicable_ops);
    // This evaluates the expanded state (again) to get preferred ops
    for (int i = 0; i < preferred_operator_heuristics.size(); i++) {
        Heuristic *h = preferred_operator_heuristics[i];
        h->evaluate(s);
        if (!h->is_dead_end()) {
            // In an alternation search with unreliable heuristics, it is
            // possible that this heuristic considers the state a dead end.
            vector<const Operator *> preferred;
            h->get_preferred_operators(preferred);
            preferred_ops.insert(preferred.begin(), preferred.end());
        }
    }
    search_progress.inc_evaluations(preferred_operator_heuristics.size());

    for (int i = 0; i < applicable_ops.size(); i++) {
        const Operator *op = applicable_ops[i];

        if ((node.get_real_g() + op->get_cost()) >= bound)
            continue;

        State succ_state(s, *op);
        search_progress.inc_generated();
        bool is_preferred = (preferred_ops.find(op) != preferred_ops.end());

        SearchNode succ_node = search_space.get_node(succ_state);

        // Previously encountered dead end. Don't re-evaluate.
        if (succ_node.is_dead_end())
            continue;

        // update new path
        if (use_multi_path_dependence || succ_node.is_new()) {
            bool h_is_dirty = false;
            for (size_t i = 0; i < heuristics.size(); i++)
                h_is_dirty = h_is_dirty || heuristics[i]->reach_state(
                    s, *op, succ_node.get_state());
            if (h_is_dirty && use_multi_path_dependence)
                succ_node.set_h_dirty();
        }

        if (succ_node.is_new()) {
            // We have not seen this state before.
            // Evaluate and create a new node.
            for (size_t i = 0; i < heuristics.size(); i++)
                heuristics[i]->evaluate(succ_state);
            succ_node.clear_h_dirty();
            search_progress.inc_evaluated_states();
            search_progress.inc_evaluations(heuristics.size());

            // Note that we cannot use succ_node.get_g() here as the
            // node is not yet open. Furthermore, we cannot open it
            // before having checked that we're not in a dead end. The
            // division of responsibilities is a bit tricky here -- we
            // may want to refactor this later.
            m_local_open_list->evaluate(node.get_g() + get_adjusted_cost(*op), is_preferred);
            bool dead_end = m_local_open_list->is_dead_end();
            if (dead_end) {
                succ_node.mark_as_dead_end();
                continue;
            }

            //TODO:CR - add an ID to each state, and then we can use a vector to save per-state information
            int succ_h = heuristics[0]->get_heuristic();
            if (do_pathmax) {
                if ((node.get_h() - get_adjusted_cost(*op)) > succ_h) {
                    //cout << "Pathmax correction: " << succ_h << " -> " << node.get_h() - get_adjusted_cost(*op) << endl;
                    succ_h = node.get_h() - get_adjusted_cost(*op);
                    heuristics[0]->set_evaluator_value(succ_h);
                    search_progress.inc_pathmax_corrections();
                }
            }
            succ_node.open(succ_h, node, op);

            m_local_open_list->insert(succ_node.get_state_buffer());
            if (search_progress.check_h_progress(succ_node.get_g())) {
                reward_progress();
            }
        } else if (succ_node.get_g() > node.get_g() + get_adjusted_cost(*op)) {
            // We found a new cheapest path to an open or closed state.
            if (reopen_closed_nodes) {
                //TODO:CR - test if we should add a reevaluate flag and if it helps
                // if we reopen closed nodes, do that
                if (succ_node.is_closed()) {
                    /* TODO: Verify that the heuristic is inconsistent.
                     * Otherwise, this is a bug. This is a serious
                     * assertion because it can show that a heuristic that
                     * was thought to be consistent isn't. Therefore, it
                     * should be present also in release builds, so don't
                     * use a plain assert. */
                    //TODO:CR - add a consistent flag to heuristics, and add an assert here based on it
                    search_progress.inc_reopened();
                }
                succ_node.reopen(node, op);
                heuristics[0]->set_evaluator_value(succ_node.get_h());
                // TODO: this appears fishy to me. Why is here only heuristic[0]
                // involved? Is this still feasible in the current version?
                m_local_open_list->evaluate(succ_node.get_g(), is_preferred);

                m_local_open_list->insert(succ_node.get_state_buffer());
            } else {
                // if we do not reopen closed nodes, we just update the parent pointers
                // Note that this could cause an incompatibility between
                // the g-value and the actual path that is traced back
                succ_node.update_parent(node, op);
            }
        }
    }

    return IN_PROGRESS;
}


void GeneralTwoLevelMacroRwEagerBestFirstSearch::initialize_local_setting()
{
    m_local_open_list->clear();
    assert(m_local_open_list->Size() == 0);

    m_isFirstNode = true;

    m_preBestHeuristicValue.clear();
    const vector<int> &best_values = search_progress.get_best_heuristic_values();
    for (size_t i = 0; i < best_values.size(); ++i){
        //cout << "PreBestValue-" << i << ": " << best_values[i] << endl;
        m_preBestHeuristicValue.push_back(best_values[i]);
    }
}


pair<SearchNode, bool> GeneralTwoLevelMacroRwEagerBestFirstSearch::fetch_local_next_node()
{
    /* TODO: The bulk of this code deals with multi-path dependence,
       which is a bit unfortunate since that is a special case that
       makes the common case look more complicated than it would need
       to be. We could refactor this by implementing multi-path
       dependence as a separate search algorithm that wraps the "usual"
       search algorithm and adds the extra processing in the desired
       places. I think this would lead to much cleaner code. */

    while (true) {
        if (m_isFirstNode){
            m_isFirstNode = false;
            return super::fetch_next_node();
        }
        if (m_local_open_list->empty()) {
            //cout << "Completely explored local state space -- no solution!" << endl;
            return make_pair(search_space.get_node(*g_initial_state), false);
        }

        vector<int> last_key_removed;
        State state(m_local_open_list->remove_min(
                        use_multi_path_dependence ? &last_key_removed : 0));
        SearchNode node = search_space.get_node(state);

        if (node.is_closed())
            continue;

        if (use_multi_path_dependence) {
            assert(last_key_removed.size() == 2);
            int pushed_h = last_key_removed[1];
            assert(node.get_h() >= pushed_h);
            if (node.get_h() > pushed_h) {
                // cout << "LM-A* skip h" << endl;
                continue;
            }
            assert(node.get_h() == pushed_h);
            if (!node.is_closed() && node.is_h_dirty()) {
                for (size_t i = 0; i < heuristics.size(); i++)
                    heuristics[i]->evaluate(node.get_state());
                node.clear_h_dirty();
                search_progress.inc_evaluations(heuristics.size());

                m_local_open_list->evaluate(node.get_g(), false);
                bool dead_end = m_local_open_list->is_dead_end();
                if (dead_end) {
                    node.mark_as_dead_end();
                    continue;
                }
                int new_h = heuristics[0]->get_heuristic();
                if (new_h > node.get_h()) {
                    assert(node.is_open());
                    node.increase_h(new_h);
                    m_local_open_list->insert(node.get_state_buffer());
                    continue;
                }
            }
        }

        node.close();
        assert(!node.is_dead_end());
        update_jump_statistic(node);
        search_progress.inc_expanded();
        return make_pair(node, true);
    }
}


bool GeneralTwoLevelMacroRwEagerBestFirstSearch::check_improvement()
{
    bool improved = false;

    const vector<int> &best_values = search_progress.get_best_heuristic_values();
    assert(best_values.size() == m_preBestHeuristicValue.size());
    for (int i = 0; i < best_values.size(); ++i)
        if (m_preBestHeuristicValue[i] == -1 || best_values[i] < m_preBestHeuristicValue[i])
        {
            improved = true;
            break;
        }

    return improved;
}



SearchEngine *GeneralTwoLevelMacroRwEagerBestFirstSearch::create_greedy(
    const vector<string> &config, int start, int &end, bool dry_run) {
    if (config[start + 1] != "(")
        throw ParseError(start + 1);

    SearchEngineOptions common_options;
    vector<ScalarEvaluator *> evals;
    OptionParser::instance()->parse_scalar_evaluator_list(config, start + 2,
                                                          end, false, evals,
                                                          dry_run);
    if (evals.empty())
        throw ParseError(end);
    end++;

    vector<Heuristic *> preferred_list;
    vector<Heuristic *> rw_h_list;
    int boost = 0;
    bool reopen_closed = false;
    int rw_mode = MRW_Parameters::PURE;
    int walk_len = 1;

    if (config[end] != ")") {
        end++;
        NamedOptionParser option_parser;
        common_options.add_options_to_parser(option_parser);
        option_parser.add_heuristic_list_option("preferred",
                                                &preferred_list, "use preferred operators of these heuristics");
        option_parser.add_heuristic_list_option("rwh",
                                                &rw_h_list, "heuristic for random walk");
        option_parser.add_int_option("boost", &boost,
                                     "boost value for successful sub-open-lists");
        option_parser.add_int_option("rw_mode", &rw_mode,
                                     "rw_mode for random walks: 0 for PURE, 2 for MHA");
        option_parser.add_int_option("walk_len", &walk_len,
                                     "walk_len for random walks, default 1");
        option_parser.parse_options(config, end, end, dry_run);
        end++;
    }
    if (config[end] != ")")
        throw ParseError(end);

    GeneralTwoLevelMacroRwEagerBestFirstSearch *engine = 0;
    if (!dry_run) {
        OpenList<state_var_t *> *open;
        OpenList<state_var_t *> *local_open;
        if ((evals.size() == 1) && preferred_list.empty()) {
            open = new StandardScalarOpenList<state_var_t *>(evals[0], false);
            local_open = new StandardScalarOpenList<state_var_t *>(evals[0], false);
        } else {
            vector<OpenList<state_var_t *> *> inner_lists;
            vector<OpenList<state_var_t *> *> local_inner_lists;
            for (int i = 0; i < evals.size(); i++) {
                inner_lists.push_back(new StandardScalarOpenList<
                        state_var_t *> (evals[i], false));
                local_inner_lists.push_back(new StandardScalarOpenList<
                        state_var_t *> (evals[i], false));
                if (!preferred_list.empty()) {
                    inner_lists.push_back(
                        new StandardScalarOpenList<state_var_t *>(evals[i],
                                                                  true));
                    local_inner_lists.push_back(new StandardScalarOpenList<
                            state_var_t *> (evals[i], true));
                }
            }
            open = new AlternationOpenList<state_var_t *>(inner_lists, boost);
            local_open = new AlternationOpenList<state_var_t *>(local_inner_lists, boost);
        }

        engine = new GeneralTwoLevelMacroRwEagerBestFirstSearch(
            common_options, open, local_open,
            false, false, false, NULL);
        engine->set_pref_operator_heuristics(preferred_list);
        engine->set_mrw_heuristic(rw_h_list);
        engine->set_mrw_walk_mode(rw_mode);
        engine->set_mrw_walk_len(walk_len); 
    }
    return engine;
}


void GeneralTwoLevelMacroRwEagerBestFirstSearch::set_mrw_walk_mode(int mode)
{
    m_mrwParam.walk_type = mode;
}

void GeneralTwoLevelMacroRwEagerBestFirstSearch::set_mrw_walk_len(int walk_len)
{
    initial_walk_len = walk_len;
}

void GeneralTwoLevelMacroRwEagerBestFirstSearch::set_mrw_heuristic(vector<Heuristic *> &heur)
{
    mrw_heuristics = heur;
}

int GeneralTwoLevelMacroRwEagerBestFirstSearch::expand_the_path(vector<const Operator*> path)
{
/**
 * add_path_step() for current_state
 *
 * for loop
 *      set predesssor ..... current state, current operators...
 *      add_path_step()
 * end loop
 */
    add_path_step();

//    State tmp_state = current_state;
    for (int i = 0; i < path.size(); ++i) {

        /**
         * part 1, environment
         */
        const Operator* op = path[i];
        m_current_state = State(m_current_state, *op);

        /**
         * part 2, add path step()
         */
        add_path_step();
    }

    return 0;
}

int GeneralTwoLevelMacroRwEagerBestFirstSearch::add_path_step()
{
    SearchNode node = search_space.get_node(m_current_state);
    State s = m_current_state;
    if (check_goal_and_set_plan(s))
        return SOLVED;

    vector<const Operator *> applicable_ops;
    set<const Operator *> preferred_ops;

    g_successor_generator->generate_applicable_ops(s, applicable_ops);
    // This evaluates the expanded state (again) to get preferred ops
    for (int i = 0; i < preferred_operator_heuristics.size(); i++) {
        Heuristic *h = preferred_operator_heuristics[i];
        h->evaluate(s);
        if (!h->is_dead_end()) {
            // In an alternation search with unreliable heuristics, it is
            // possible that this heuristic considers the state a dead end.
            vector<const Operator *> preferred;
            h->get_preferred_operators(preferred);
            preferred_ops.insert(preferred.begin(), preferred.end());
        }
    }
    search_progress.inc_evaluations(preferred_operator_heuristics.size());

    for (int i = 0; i < applicable_ops.size(); i++) {
        const Operator *op = applicable_ops[i];

        if ((node.get_real_g() + op->get_cost()) >= bound)
            continue;

        State succ_state(s, *op);
        search_progress.inc_generated();
        bool is_preferred = (preferred_ops.find(op) != preferred_ops.end());

        SearchNode succ_node = search_space.get_node(succ_state);

        // Previously encountered dead end. Don't re-evaluate.
        if (succ_node.is_dead_end())
            continue;

        // update new path
        if (use_multi_path_dependence || succ_node.is_new()) {
            bool h_is_dirty = false;
            for (size_t i = 0; i < heuristics.size(); i++)
                h_is_dirty = h_is_dirty || heuristics[i]->reach_state(
                    s, *op, succ_node.get_state());
            if (h_is_dirty && use_multi_path_dependence)
                succ_node.set_h_dirty();
        }

        if (succ_node.is_new()) {
            // We have not seen this state before.
            // Evaluate and create a new node.
            for (size_t i = 0; i < heuristics.size(); i++)
                heuristics[i]->evaluate(succ_state);
            succ_node.clear_h_dirty();
            search_progress.inc_evaluated_states();
            search_progress.inc_evaluations(heuristics.size());

            // Note that we cannot use succ_node.get_g() here as the
            // node is not yet open. Furthermore, we cannot open it
            // before having checked that we're not in a dead end. The
            // division of responsibilities is a bit tricky here -- we
            // may want to refactor this later.
            m_local_open_list->evaluate(node.get_g() + get_adjusted_cost(*op), is_preferred);
            bool dead_end = m_local_open_list->is_dead_end();
            if (dead_end) {
                succ_node.mark_as_dead_end();
                continue;
            }

            //TODO:CR - add an ID to each state, and then we can use a vector to save per-state information
            int succ_h = heuristics[0]->get_heuristic();
            if (do_pathmax) {
                if ((node.get_h() - get_adjusted_cost(*op)) > succ_h) {
                    //cout << "Pathmax correction: " << succ_h << " -> " << node.get_h() - get_adjusted_cost(*op) << endl;
                    succ_h = node.get_h() - get_adjusted_cost(*op);
                    heuristics[0]->set_evaluator_value(succ_h);
                    search_progress.inc_pathmax_corrections();
                }
            }
            succ_node.open(succ_h, node, op);

            m_local_open_list->insert(succ_node.get_state_buffer());
            if (search_progress.check_h_progress(succ_node.get_g())) {
                reward_progress();
            }
        } else if (succ_node.get_g() > node.get_g() + get_adjusted_cost(*op)) {
            // We found a new cheapest path to an open or closed state.
            if (reopen_closed_nodes) {
                //TODO:CR - test if we should add a reevaluate flag and if it helps
                // if we reopen closed nodes, do that
                if (succ_node.is_closed()) {
                    /* TODO: Verify that the heuristic is inconsistent.
                     * Otherwise, this is a bug. This is a serious
                     * assertion because it can show that a heuristic that
                     * was thought to be consistent isn't. Therefore, it
                     * should be present also in release builds, so don't
                     * use a plain assert. */
                    //TODO:CR - add a consistent flag to heuristics, and add an assert here based on it
                    search_progress.inc_reopened();
                }
                succ_node.reopen(node, op);
                heuristics[0]->set_evaluator_value(succ_node.get_h());
                // TODO: this appears fishy to me. Why is here only heuristic[0]
                // involved? Is this still feasible in the current version?
                m_local_open_list->evaluate(succ_node.get_g(), is_preferred);

                m_local_open_list->insert(succ_node.get_state_buffer());
            } else {
                // if we do not reopen closed nodes, we just update the parent pointers
                // Note that this could cause an incompatibility between
                // the g-value and the actual path that is traced back
                succ_node.update_parent(node, op);
            }
        }
    }

    return IN_PROGRESS;
}



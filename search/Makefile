## Set USE_LP to 1 to enable linear programming stuff.
USE_LP=0
STATE_VAR_BYTES=1

HEADERS = \
          axioms.h \
          best_first_search.h \
          causal_graph.h \
          closed_list.h \
          combining_evaluator.h \
          domain_transition_graph.h \
          enforced_hill_climbing_search.h \
          exact_timer.h \
          g_evaluator.h \
          general_eager_best_first_search.h \
          general_lazy_best_first_search.h \
          globals.h \
          heuristic.h \
          ipc_max_heuristic.h \
          iterated_search.h \
          max_evaluator.h \
          operator.h \
          operator_cost.h \
          option_parser.h \
          plugin.h \
          pref_evaluator.h \
          relaxation_heuristic.h \
          search_engine.h \
          search_node_info.h \
          search_progress.h \
          search_space.h \
          state.h \
          successor_generator.h \
          sum_evaluator.h \
          timer.h \
          string_utils.h \
          mrw_parameters.h \
          walker.h \
		  general_macro_lazy_best_first_search.h \
		  general_two_level_macro_lazy_best_first_search.h \
		  general_two_level_macro_lazy_best_first_search_for_visitall.h \
		  general_lazy_best_first_search_understand_preferred_op.h \
		  general_path_h_stats_lazy_best_first_search.h	   \
		  general_count_discrepancy_lazy_best_first_search.h	   \
		  general_two_level_eager_best_first_search.h	   \
		  general_eager_best_first_search_pref_op_revisited.h \
		  general_two_level_lazy_best_first_search_pref_op_revisited.h \
		  general_two_level_macro_rw_lazy_best_first_search.h \
		  general_two_level_macro_rw_ls_mix_lazy_best_first_search.h \
		  general_two_level_macro_rw_eager_best_first_search.h \
		  iterated_search_aras.h \
          iterated_search_aras_rw.h \
		  xml_translator.h \
		  utilities.h \
          weighted_evaluator.h \
          UctParam.h \
          \
          open_lists/alternation_open_list.h \
          open_lists/open_list_buckets.h \
          open_lists/pareto_open_list.h \
          open_lists/standard_scalar_open_list.h \
          open_lists/tiebreaking_open_list.h \
          open_lists/standard_scalar_epsilon_open_list.h \
          open_lists/tunable_alternation_open_list.h \
          open_lists/diverse_open_list.h \
          open_lists/probabilistic_open_list_m.h \

## Each of the following "HEADERS += ..." constructs defines a
## "plugin" feature that can be enabled or disabled by simply
## commenting out the respective lines in the Makefile.

## As with all changes to the Makefile, you will have to force a
## rebuild after such a change. Deleting the executable and running
## "make" to relink is enough; no need to do a complete rebuild.

HEADERS += additive_heuristic.h
HEADERS += blind_search_heuristic.h
HEADERS += cea_heuristic.h
HEADERS += cg_heuristic.h cg_cache.h
HEADERS += ff_heuristic.h
HEADERS += goal_count_heuristic.h
HEADERS += hm_heuristic.h
HEADERS += lm_cut_heuristic.h
HEADERS += max_heuristic.h

# HEADERS += mas_heuristic.h \
#            abstraction.h \
#            operator_registry.h \
#            variable_order_finder.h

HEADERS += raz_mas_heuristic.h \
           raz_abstraction.h \
           raz_operator_registry.h \
           raz_variable_order_finder.h

HEADERS += landmarks/exploration.h \
           landmarks/h_m_landmarks.h \
           landmarks/lama_ff_synergy.h \
           landmarks/landmark_cost_assignment.h \
           landmarks/landmark_count_heuristic.h \
           landmarks/landmark_status_manager.h \
           landmarks/landmarks_graph_merged.h \
           landmarks/landmarks_graph.h \
           landmarks/landmarks_graph_rpg_exhaust.h \
           landmarks/landmarks_graph_rpg_sasp.h \
           landmarks/landmarks_graph_rpg_search.h \
           landmarks/landmarks_graph_zhu_givan.h \
           landmarks/util.h

HEADERS += learning/AODE.h \
           learning/classifier.h \
           learning/composite_feature_extractor.h \
           learning/feature_extractor.h \
           learning/maximum_heuristic.h \
           learning/naive_bayes_classifier.h \
           learning/PDB_state_space_sample.h \
           learning/probe_state_space_sample.h \
           learning/selective_max_heuristic.h \
           learning/state_space_sample.h \
           learning/state_vars_feature_extractor.h

SOURCES = planner.cc $(HEADERS:%.h=%.cc)
TARGET  = downward

ARGS_PROFILE = --search 'astar(lmcut())' < profile-input.pre



OBJECT_SUFFIX_RELEASE = .$(STATE_VAR_BYTES)
TARGET_SUFFIX_RELEASE = -$(STATE_VAR_BYTES)
OBJECT_SUFFIX_DEBUG   = .$(STATE_VAR_BYTES).debug
TARGET_SUFFIX_DEBUG   = -$(STATE_VAR_BYTES)-debug
OBJECT_SUFFIX_PROFILE = .$(STATE_VAR_BYTES).profile
TARGET_SUFFIX_PROFILE = -$(STATE_VAR_BYTES)-profile

OBJECTS_RELEASE = $(SOURCES:%.cc=.obj/%$(OBJECT_SUFFIX_RELEASE).o)
TARGET_RELEASE  = $(TARGET)$(TARGET_SUFFIX_RELEASE)

OBJECTS_DEBUG   = $(SOURCES:%.cc=.obj/%$(OBJECT_SUFFIX_DEBUG).o)
TARGET_DEBUG    = $(TARGET)$(TARGET_SUFFIX_DEBUG)

OBJECTS_PROFILE = $(SOURCES:%.cc=.obj/%$(OBJECT_SUFFIX_PROFILE).o)
TARGET_PROFILE  = $(TARGET)$(TARGET_SUFFIX_PROFILE)


CC     = g++
DEPEND = g++ -MM

## CCOPT, LINKOPT, POSTLINKOPT are options for compiler and linker
## that are used for all three targets (release, debug, and profile).
## (POSTLINKOPT are options that appear *after* all object files.)

CCOPT  = -g
CCOPT += -m64
CCOPT +=
CCOPT += -Wall -W -Wno-sign-compare -Wno-deprecated -ansi -pedantic -DSTATE_VAR_BYTES=$(STATE_VAR_BYTES)

## The following line is a HACK to cross-compile a 64-bit executable
## on a 32-bit Ubuntu machine (it also needs the -m64 to be changed to
## -m64, of course.) See http://stackoverflow.com/questions/4643197/missing-include-bits-cconfig-h-when-cross-compiling-64-bit-program-on-32-bit.
CCOPT += -I/usr/include/c++/4.4/i686-linux-gnu

LINKOPT  = -g
LINKOPT += -m64
LINKOPT +=

POSTLINKOPT =

## Additional specialized options for the various targets follow.
## In release mode, we link statically since this makes it more likely
## that local compiles will work on the various grids (gkigrid, Black
## Forest Grid).
##
## NOTE: This precludes some uses of exceptions.
##        For details, see man gcc on -static-libgcc.

CCOPT_RELEASE  = -O3 -DNDEBUG -fomit-frame-pointer
CCOPT_DEBUG    = -O3
CCOPT_PROFILE  = -O3 -pg

LINKOPT_RELEASE  = -static -static-libgcc
LINKOPT_DEBUG    = -lrt
LINKOPT_PROFILE  = -pg -lrt
# LINKOPT_PROFILE += -lc_p

POSTLINKOPT_RELEASE  = -Wl,-Bstatic -lrt
POSTLINKOPT_DEBUG    =
POSTLINKOPT_PROFILE  =

## Define the default target up here so that the LP stuff below
## doesn't define a default target.

default: release

ifeq ($(USE_LP),1)

COIN_ROOT = lp/coin

$(COIN_ROOT):
	cd lp && ./setup

$(TARGET_RELEASE) $(OBJECTS_RELEASE): $(COIN_ROOT)
$(TARGET_DEBUG) $(OBJECTS_DEBUG): $(COIN_ROOT)
$(TARGET_PROFILE) $(OBJECTS_PROFILE): $(COIN_ROOT)

## We want to link the Linear Programming libraries statically since
## they are unlikely to be preinstalled on the grids we use for
## evaluation. Static linking is a bit tricky: we need to specify the
## libraries *after* the source files and in such an order that if A
## depends on B, A is listed before B. (In case of dependency cycles,
## we can and must list them multiple times.) The following set of
## libraries and their ordering have been determined experimentally
## and hence might break if we use more functions from the LP
## libraries. See
## http://ask.metafilter.com/117792/How-to-fix-C-static-linking-problems

COIN_LIBS = OsiClp Clp CoinUtils Osi

## We want to link the COIN libraries statically and link other
## libraries dynamically, unless we are in release mode. We accomplish
## this by using -Wl,-Bstatic before the COIN libs and -Wl,-Bdynamic
## afterwards (unless in release mode). See
## http://ubuntuforums.org/showthread.php?t=491455

COIN_CCOPT = -I $(COIN_ROOT)/include/coin -D USE_LP -D COIN_USE_CLP
COIN_LINKOPT = -L $(COIN_ROOT)/lib -Wl,-Bstatic $(COIN_LIBS:%=-l %)

CCOPT += $(COIN_CCOPT)

POSTLINKOPT_RELEASE += $(COIN_LINKOPT)
POSTLINKOPT_DEBUG   += $(COIN_LINKOPT) -Wl,-Bdynamic
POSTLINKOPT_PROFILE += $(COIN_LINKOPT) -Wl,-Bdynamic

endif

SHELL = /bin/bash

all: release debug profile

## Build rules for the release target follow.

release: $(TARGET_RELEASE)

$(TARGET_RELEASE): $(OBJECTS_RELEASE)
	$(CC) $(LINKOPT) $(LINKOPT_RELEASE) $(OBJECTS_RELEASE) $(POSTLINKOPT) $(POSTLINKOPT_RELEASE) -o $(TARGET_RELEASE)

$(OBJECTS_RELEASE): .obj/%$(OBJECT_SUFFIX_RELEASE).o: %.cc
	@mkdir -p $$(dirname $@)
	$(CC) $(CCOPT) $(CCOPT_RELEASE) -c $< -o $@

## Build rules for the debug target follow.

debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJECTS_DEBUG)
	$(CC) $(LINKOPT) $(LINKOPT_DEBUG) $(OBJECTS_DEBUG) $(POSTLINKOPT) $(POSTLINKOPT_DEBUG) -o $(TARGET_DEBUG)

$(OBJECTS_DEBUG): .obj/%$(OBJECT_SUFFIX_DEBUG).o: %.cc
	@mkdir -p $$(dirname $@)
	$(CC) $(CCOPT) $(CCOPT_DEBUG) -c $< -o $@

## Build rules for the profile target follow.

profile: $(TARGET_PROFILE)

$(TARGET_PROFILE): $(OBJECTS_PROFILE)
	$(CC) $(LINKOPT) $(LINKOPT_PROFILE) $(OBJECTS_PROFILE) $(POSTLINKOPT) $(POSTLINKOPT_PROFILE) -o $(TARGET_PROFILE)

$(OBJECTS_PROFILE): .obj/%$(OBJECT_SUFFIX_PROFILE).o: %.cc
	@mkdir -p $$(dirname $@)
	$(CC) $(CCOPT) $(CCOPT_PROFILE) -c $< -o $@

## Additional targets follow.

PROFILE: $(TARGET_PROFILE)
	./$(TARGET_PROFILE) $(ARGS_PROFILE)
	gprof $(TARGET_PROFILE) | (cleanup-profile 2> /dev/null || cat) > PROFILE

clean:
	rm -rf .obj
	rm -f *~ *.pyc
	rm -f Makefile.depend gmon.out PROFILE core
	rm -f sas_plan

distclean: clean
	rm -f $(TARGET_RELEASE) $(TARGET_DEBUG) $(TARGET_PROFILE)

## Note: If we just call gcc -MM on a source file that lives within a
## subdirectory, it will strip the directory part in the output. Hence
## the for loop with the sed call.

Makefile.depend: $(SOURCES) $(HEADERS)
	rm -f Makefile.temp
	for source in $(SOURCES) ; do \
	    $(DEPEND) $$source > Makefile.temp0; \
	    objfile=$${source%%.cc}.o; \
	    sed -i -e "s@^[^:]*:@$$objfile:@" Makefile.temp0; \
	    cat Makefile.temp0 >> Makefile.temp; \
	done
	rm -f Makefile.temp0 Makefile.depend
	sed -e "s@\(.*\)\.o:\(.*\)@.obj/\1$(OBJECT_SUFFIX_RELEASE).o:\2@" Makefile.temp >> Makefile.depend
	sed -e "s@\(.*\)\.o:\(.*\)@.obj/\1$(OBJECT_SUFFIX_DEBUG).o:\2@" Makefile.temp >> Makefile.depend
	sed -e "s@\(.*\)\.o:\(.*\)@.obj/\1$(OBJECT_SUFFIX_PROFILE).o:\2@" Makefile.temp >> Makefile.depend
	rm -f Makefile.temp

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include Makefile.depend
endif
endif

.PHONY: default all release debug profile clean distclean

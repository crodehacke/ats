# -*- mode: cmake -*-

#
#  ATS
#  Thaw depth for dynamic subgrid (surface flow) model
#

#include_directories(${Amanzi_TPL_MSTK_INCLUDE_DIRS})
#add_definitions("-DMSTK_HAVE_MPI")


add_library(flow_relations_thaw_depth
  thaw_depth_evaluator.cc	
  max_thaw_depth_evaluator.cc	
  )

install(TARGETS flow_relations_thaw_depth DESTINATION lib)





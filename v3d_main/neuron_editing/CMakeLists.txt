set(NeuronEditing_SRCS
  apo_xforms.cpp
  neuron_format_converter.cpp
  neuron_sim_scores.cpp
  neuron_xforms.cpp
  v_neuronswc.cpp
  )

add_library( neuron_editing ${NeuronEditing_SRCS} )

target_link_libraries( neuron_editing )


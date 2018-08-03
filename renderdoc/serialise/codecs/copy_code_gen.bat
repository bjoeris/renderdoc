del sample_cpp_trace\gen_*
del sample_cpp_trace\buffer_*
del sample_cpp_trace\shader_*
del sample_cpp_trace\pipeline_cache_*

copy ..\..\..\renderdoccmd\gen_* sample_cpp_trace\gen_*
copy ..\..\..\renderdoccmd\buffer_* sample_cpp_trace\buffer_*
copy ..\..\..\renderdoccmd\shader_* sample_cpp_trace\shader_*
copy ..\..\..\renderdoccmd\pipeline_cache_* sample_cpp_trace\pipeline_cache_*

del ..\..\..\renderdoccmd\gen_*
del ..\..\..\renderdoccmd\buffer_*
del ..\..\..\renderdoccmd\shader_*
del ..\..\..\renderdoccmd\pipeline_cache_*
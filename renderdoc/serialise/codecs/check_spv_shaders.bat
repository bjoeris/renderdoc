for %%x in (sample_cpp_trace\shader_*) do "%VULKAN_SDK%\Bin\spirv-val.exe" %%x 
for %%x in (sample_cpp_trace\shader_*) do "%VULKAN_SDK%\Bin\spirv-dis.exe" -o %%x.asm %%x
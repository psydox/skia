               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %_0_v "_0_v"                      ; id %23
               OpName %_1_x "_1_x"                      ; id %30
               OpName %_2_y "_2_y"                      ; id %33
               OpName %_3_z "_3_z"                      ; id %35
               OpName %_4_w "_4_w"                      ; id %37
               OpName %a "a"                            ; id %39
               OpName %_9_x "_9_x"                      ; id %41
               OpName %_10_y "_10_y"                    ; id %45
               OpName %_11_z "_11_z"                    ; id %49
               OpName %_12_w "_12_w"                    ; id %53
               OpName %b "b"                            ; id %57
               OpName %c "c"                            ; id %59

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %_0_v RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %_1_x RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %_2_y RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %_3_z RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %_4_w RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %_9_x RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %_10_y RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %_11_z RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %_12_w RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %63 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %69 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
       %_0_v =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
       %_1_x =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
       %_2_y =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
       %_3_z =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
       %_4_w =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
       %_9_x =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
      %_10_y =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
      %_11_z =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
      %_12_w =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %c =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %82 =   OpVariable %_ptr_Function_v4float Function
         %25 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 =   OpLoad %v4float %25                ; RelaxedPrecision
                 OpStore %_0_v %29
         %32 =   OpCompositeExtract %float %29 0    ; RelaxedPrecision
                 OpStore %_1_x %32
         %34 =   OpCompositeExtract %float %29 1    ; RelaxedPrecision
                 OpStore %_2_y %34
         %36 =   OpCompositeExtract %float %29 2    ; RelaxedPrecision
                 OpStore %_3_z %36
         %38 =   OpCompositeExtract %float %29 3    ; RelaxedPrecision
                 OpStore %_4_w %38
         %40 =   OpCompositeConstruct %v4float %32 %34 %36 %38  ; RelaxedPrecision
                 OpStore %a %40
         %42 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %43 =   OpLoad %v4float %42                ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 0    ; RelaxedPrecision
                 OpStore %_9_x %44
         %46 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
         %48 =   OpCompositeExtract %float %47 1    ; RelaxedPrecision
                 OpStore %_10_y %48
         %50 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %51 =   OpLoad %v4float %50                ; RelaxedPrecision
         %52 =   OpCompositeExtract %float %51 2    ; RelaxedPrecision
                 OpStore %_11_z %52
         %54 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %55 =   OpLoad %v4float %54                ; RelaxedPrecision
         %56 =   OpCompositeExtract %float %55 3    ; RelaxedPrecision
                 OpStore %_12_w %56
         %58 =   OpCompositeConstruct %v4float %44 %48 %52 %56  ; RelaxedPrecision
                 OpStore %b %58
                 OpStore %c %63
         %70 =   OpFOrdEqual %v4bool %40 %69
         %72 =   OpAll %bool %70
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
         %75 =       OpFOrdEqual %v4bool %58 %69
         %76 =       OpAll %bool %75
                     OpBranch %74

         %74 = OpLabel
         %77 =   OpPhi %bool %false %22 %76 %73
                 OpSelectionMerge %79 None
                 OpBranchConditional %77 %78 %79

         %78 =     OpLabel
                     OpBranch %79

         %79 = OpLabel
         %81 =   OpPhi %bool %false %74 %true %78
                 OpSelectionMerge %85 None
                 OpBranchConditional %81 %83 %84

         %83 =     OpLabel
         %86 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %88 =       OpLoad %v4float %86            ; RelaxedPrecision
                     OpStore %82 %88
                     OpBranch %85

         %84 =     OpLabel
         %89 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %91 =       OpLoad %v4float %89            ; RelaxedPrecision
                     OpStore %82 %91
                     OpBranch %85

         %85 = OpLabel
         %92 =   OpLoad %v4float %82                ; RelaxedPrecision
                 OpReturnValue %92
               OpFunctionEnd

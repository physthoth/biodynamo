/**
 * This file contains code generation customizations for class Excrescence.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (EXCRESENCE_NATIVE and EXCRESENCE_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/synapse/excresence.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"

%define %Excrescence_cx3d_shared_ptr()
  %cx3d_shared_ptr(Excrescence,
                   ini/cx3d/synapses/Excrescence,
                   cx3d::synapse::Excrescence);
%enddef

%define %Excrescence_java()
  %java_defined_class(cx3d::synapse::Excrescence,
                      Excrescence,
                      Excrescence,
                      ini.cx3d.synapses.Excrescence,
                      ini/cx3d/synapses/Excrescence);
%enddef

%define %Excrescence_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::synapse::Excrescence>,
                   Excrescence,
                   ini.cx3d.synapses.Excrescence);
%enddef

/**
 * apply customizations
 */
%Excrescence_cx3d_shared_ptr();
%Excrescence_java();
%Excrescence_stdlist();
%typemap(javaimports) cx3d::synapse::Excrescence "import ini.cx3d.swig.NativeStringBuilder;"
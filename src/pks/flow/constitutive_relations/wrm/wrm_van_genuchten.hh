/* -*-  mode: c++; indent-tabs-mode: nil -*- */
//! WRMVanGenuchten : water retention model using van Genuchten's parameterization

/*
  ATS is released under the three-clause BSD License. 
  The terms of use and "as is" disclaimer for this license are 
  provided in the top-level COPYRIGHT file.

  Authors: Ethan Coon (ecoon@lanl.gov)
*/

/*!

van Genuchten's water retention curve.

* `"alpha [Pa^-1]`" ``[double]`` van Genuchten's alpha

ONE OF:
* `"n`" ``[double]`` van Genuchten's n
OR
* `"m`" ``[double]`` van Genuchten's m, m = 1 - 1/n
END

* `"residual saturation [-]`" ``[double]`` **0.0**

* `"smoothing interval width [saturation]`" ``[double]`` **0.0**

* `"Mualem exponent l`" ``[double]`` **0.5**

* `"Krel function name`" ``[string]`` **Mualem**  `"Mualem`" or `"Burdine`"

Example:

.. code-block:: xml

    <ParameterList name="moss" type="ParameterList">
      <Parameter name="region" type="string" value="moss" />
      <Parameter name="WRM Type" type="string" value="van Genuchten" />
      <Parameter name="van Genuchten alpha" type="double" value="0.002" />
      <Parameter name="van Genuchten m" type="double" value="0.2" />
      <Parameter name="residual saturation" type="double" value="0.0" />
      <Parameter name="smoothing interval width [saturation]" type="double" value=".05" />
    </ParameterList>

*/

#ifndef ATS_FLOWRELATIONS_WRM_VAN_GENUCHTEN_
#define ATS_FLOWRELATIONS_WRM_VAN_GENUCHTEN_

#include "Teuchos_ParameterList.hpp"
#include "Spline.hh"

#include "wrm.hh"
#include "Factory.hh"

namespace Amanzi {
namespace Flow {

class WRMVanGenuchten : public WRM {

public:
  explicit WRMVanGenuchten(Teuchos::ParameterList& plist);

  // required methods from the base class
  double k_relative(double saturation);
  double d_k_relative(double saturation);
  double saturation(double pc);
  double d_saturation(double pc);
  double capillaryPressure(double saturation);
  double d_capillaryPressure(double saturation);
  double residualSaturation() { return sr_; }

 private:
  void InitializeFromPlist_();

  Teuchos::ParameterList& plist_;

  double m_;  // van Genuchten parameters: m, n, alpha
  double n_;
  double l_;
  double alpha_;
  double sr_;  // van Genuchten residual saturation

  int function_;
  double s0_;  // regularization threshold in saturation
  Amanzi::Utils::Spline fit_kr_;

  double pc0_;
  Amanzi::Utils::Spline fit_s_;
  

  static Utils::RegisteredFactory<WRM,WRMVanGenuchten> factory_;
};

} //namespace
} //namespace

#endif

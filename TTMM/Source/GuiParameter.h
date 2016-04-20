/**
 * @file GuiParameter.h
 * @author philipp.kroos@fh-bielefeld.de
 * @brief Declaration of the class @code GuiParameter
 */

#ifndef GUI_PARAMETER_H
#define GUI_PARAMETER_H

#include <string>
#include <cmath>

#include "FileWriter.h"

namespace ttmm {
    /**
     @class GUIParameter
     @brief Encapsulates a parameter for a Plugin.

     This is a container for a parameter from the standar
     VST2-parameter representation.  If no GUI is provided for a
     plugin (e.g. using the JUCE AudioProcessorEditor), a VST2 plugin
     has a default representation for each parameter: a Slider with
     range [0, 1]. This class is a convenience class too convert the
     VST2-range to the range [minValue, maxValue].  Conversion happens
     automatically during setting/getting the value, e.g. @code get will
     always return the value in range [0, 1] whereas the value stored by
     @code set will always be in range of [min, max].

     @see GeneralPluginProcessor which maps the methods of this class
     to the JUCE/VST2 interface.
     @todo Test if this works with negative ranges too?
     */
    class GUIParameter
    {

    public:
        GUIParameter() = default;
        GUIParameter(int minValue, int maxValue, int value, std::string name, std::string description)
            : min(minValue)
            , max(maxValue)
            , value(value)
            , name(name)
            , description(description)
        {
        }

        const float range() const { return float(max - min); }

	/**
	 * @brief Return the current value in VST-units.
	 * 
	 * @return @code value scaled to the range [0, 1].
	 */
        float getValue()
        {
            return static_cast<float>(value - min) / range();
        }

	/**
	 * @brief Set the current value in VST-units.
	 * 
	 * The value passed will be converted to and stored as value
	 * in the range [min, max].
	 * @see @code min, @code max.
	 * @return @code value as stored internally.
	 */
        int setValue(float betweenZeroAndOne)
        {
            value = static_cast<int>(std::round(betweenZeroAndOne * range() + min));
            //ttmm::logger.write("Translated " + std::to_string(betweenZeroAndOne) + " to ", value);
            return value;
        }

    public: // Publicly accessible values

        std::string name; ///< The name as printed on the VST-GUI
        std::string description; ///< The description as printed on the VST-GUI

    private:
        int value; ///< current value
        int min; ///< minimum value
        int max; ///< maximum value
    };

}

#endif

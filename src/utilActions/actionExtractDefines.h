#ifndef __ACTIONEXTRACTDEFINES_H__
#define __ACTIONEXTRACTDEFINES_H__

////////////////////////////////////////////////////////////////////////////////
/// \copiright RCO, 2019
///
/// This program is free software: you can redistribute it and/or modify it
///     under the terms of the GNU General Public License published by the Free
///     Software Foundation, either version 2 of the License, or (at your
///     option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
///     ANY WARRANTY; without even the implied of MERCHANTABILITY or FITNESS FOR
///     A PARTICULAR PURPOSE. See the GNU General Public License for more
///     details.
///
/// You should have received a copy of the GNU General Public License along with
///     this program. If not, see <http://www.gnu.org/licenses/>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \file actionExtractDefines.h
/// \brief action manager utility defines, this file shoud be in accordance with
///     IHM json generator
/// \copyright GPLv2
////////////////////////////////////////////////////////////////////////////////

static const char STRATEGY[] = "Strategie";
static const char STEP_NAME[] = "nomEtape";
static const char STEP_POINTS[] = "nbPoints";
static const char ACTION_ARRAY[] = "arraySequence";
static const char RATE[] = "taux";
static const char RATE_ARRAY[] = "TauxArray";
static const char RATE_CONDITINON[] = "condition";
static const char RATE_PARAMETER[] = "param";
static const char RATE_VALUE[] = "valeur";
static const char NEXT_ARRAY[] = "arrayGirl";
static const char NEXT_ID[] = "indiceFille";
static const char TIMEOUT_ARRAY[] = "arrayTimeout";
static const char TIMEOUT_ID[] = "indiceTimeout";
static const char PARAM_ARRAY[] = "arrayParam";
static const char PARAM_NAME[] = "nomParam";
static const char PARAM_VALUE[] = "defaultValue";

/// \note a json strategy file contain an obect with a \see STRATEGY, a STRATEGY
///     is an array of object each object is called STEP. Each STEP is an 
///     independent collection of action stored in ACTION_ARRAY. Each action as
///     is own parameters sotred in PARAM_ARRY
/// \exemple of strategy file
/// {
///     STRATEGY: [
///         {
///             RATE_ARRAY: [
///                 {
///                     RATE_CONDITINON: float,
///                     RATE_PARAMETER: float,
///                     RATE: float,
///                     RATE_VALUE: float
///                 }
///             ],
///             ACTION_ARRAY: [
///                 {
///                     "arrayDaddy": [
///                     ],
///                     NEXT_ARRAY: [
///                         {
///                             NEXT_ID: 1
///                         }
///                     ],
///                     PARAM_ARRAY: [
///                     ],
///                     TIMEOUT_ARRAY: [
///                     ],
///                     "blocante": false,
///                     "indice": 0,
///                     "nomAction": "Départ",
///                     "xBloc": 81,
///                     "yBloc": 299
///                 }
///             ],
///             "color": "#aa007f",
///             "dateMax": 10,
///             "deadline": 100,
///             STEP_POINTS: float,
///             STEP_NAME: "Str",
///             "tempsMax": 20,
///             "tempsMoyen": 15,
///             "xEtape": 174,
///             "yEtape": 417
///         }
///     ]
/// }

typedef enum {
	UT, // up than
	EQ, // equal
	LT // less than
}
condition_t;

typedef enum {
	TIME,
	CUPS
}
param_t;

#endif
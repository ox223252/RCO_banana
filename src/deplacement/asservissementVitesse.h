
#ifndef __ASSERVISSEMENTVITESSE_H__
#define __ASSERVISSEMENTVITESSE_H__

#include <stdint.h>
#include <stdio.h>
#include "controleMoteur.h"


////////////////////////////////////////////////////////////////////////////////
/// \fn void razAsserv ( void );
/// \brief Reinitialize speed asservissement variables to default values.
////////////////////////////////////////////////////////////////////////////////
void razAsserv();

////////////////////////////////////////////////////////////////////////////////
/// \fn void initAsservissementVitesse ( float _coeffPG, float _coeffIG, float _coeffDG,
///   int16_t _maxSpeed,float _coeffPD, float _coeffID, float _coeffDD );
/// \param[ in ] _coeffPG : Coefficient for proportionnal factor to the calculation of left speed asservissment
/// \param[ in ] _coeffIG : Coefficient for integer factor to the calculation of left speed asservissment
/// \param[ in ] _coeffDG : Coefficient for derivative factor to the calculation of left speed asservissment
/// \param[ in ] _maxSpeed : max speed allowed
/// \param[ in ] _coeffPD : Coefficient for proportionnal factor to the calculation of right speed asservissment
/// \param[ in ] _coeffID : Coefficient for integer factor to the calculation of right speed asservissment
/// \param[ in ] _coeffDD : Coefficient for derivative factor to the calculation of right speed asservissment
////////////////////////////////////////////////////////////////////////////////
void initAsservissementVitesse(const float _coeffPG,const float _coeffIG,const float _coeffDG,const int16_t _maxSpeed,const float _coeffPD,const float _coeffID,const float _coeffDD);

////////////////////////////////////////////////////////////////////////////////
/// \fn void initAsservissementVitesse ( float _coeffPG, float _coeffIG, float _coeffDG,
///   int16_t _maxSpeed,float _coeffPD, float _coeffID, float _coeffDD );
/// \param[ in ] consigneGauche : asked speed of the right motor in mm/s
/// \param[ in ] consigneDroite : asked speed of the right motor in mm/s
/// \param[ in ] vitesseGauche : actual speed of the left motor in mm/s
/// \param[ in ] vitesseDroite : actual speed of the right motor in mm/s
////////////////////////////////////////////////////////////////////////////////
int asservirVitesseGaucheDroite(const int16_t consigneGauche,const int16_t consigneDroite,const int16_t vitesseGauche,const int16_t vitesseDroite);

////////////////////////////////////////////////////////////////////////////////
/// \fn void arreteTesMoteursSimone (  );
/// Bien nommée.
////////////////////////////////////////////////////////////////////////////////
void arreteTesMoteursSimone();


#endif //__ASSERVISSEMENTVITESSE_H__

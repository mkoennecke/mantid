/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Extension to the Qwt Widget Library
 * Copyright (C) 2013   RAL & ORNL
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_SCALE_ENGINE_SQUARE_H
#define QWT_SCALE_ENGINE_SQUARE_H


#include "WidgetDllOption.h"
#include "qwt_global.h"
#include "qwt_scale_div.h"
#include "qwt_double_interval.h"
#include "qwt_scale_engine.h"

class QwtScaleTransformation;

/*!
  \brief A scale engine for Square scales

  The step size will fit into the pattern 
  \f$\left\{ 1,2,5\right\} \cdot 10^{n}\f$, where n is an integer.
*/

class EXPORT_OPT_MANTIDQT_MANTIDWIDGETS QwtSquareScaleEngine: public QwtScaleEngine
{
public:
    virtual void autoScale(int maxSteps, 
        double &x1, double &x2, double &stepSize) const;

    virtual QwtScaleDiv divideScale(double x1, double x2,
        int numMajorSteps, int numMinorSteps,
        double stepSize = 0.0) const;

    virtual QwtScaleTransformation *transformation() const;

protected:
    QwtDoubleInterval align(const QwtDoubleInterval&,
        double stepSize) const;

private:
    void buildTicks(
        const QwtDoubleInterval &, double stepSize, int maxMinSteps,
        QwtValueList ticks[QwtScaleDiv::NTickTypes]) const;

    void buildMinorTicks(
        const QwtValueList& majorTicks,
        int maxMinMark, double step,
        QwtValueList &, QwtValueList &) const;

    QwtValueList buildMajorTicks(
        const QwtDoubleInterval &interval, double stepSize) const;
};


#endif

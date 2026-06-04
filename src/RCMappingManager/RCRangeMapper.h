#pragma once

#include <QObject>

class RCRangeMapper : public QObject {
    Q_OBJECT

public:
    RCRangeMapper() = default;
    RCRangeMapper(float inMin, float inMax, float outMin = 1000.0f, float outMax = 2000.0f);

    Q_PROPERTY(float inMin               READ inMin              NOTIFY inMinChanged)
    Q_PROPERTY(float inMax               READ inMax              NOTIFY inMaxChanged)
    Q_PROPERTY(float inRange             READ inRange            NOTIFY inRangeChanged)
    Q_PROPERTY(float outMin              READ outMin             NOTIFY outMinChanged)
    Q_PROPERTY(float outMax              READ outMax             NOTIFY outMaxChanged)
    Q_PROPERTY(float outRange            READ outRange           NOTIFY outRangeChanged)
    Q_PROPERTY(float rawValue            READ rawValue           NOTIFY rawValueChanged)
    Q_PROPERTY(float normalizedValue     READ normalizedValue    NOTIFY normalizedValueChanged)
    Q_PROPERTY(float mappedValue         READ mappedValue        NOTIFY mappedValueChanged)
    Q_PROPERTY(bool  IsClamping    READ IsClamping   NOTIFY IsClampingChanged)
    Q_PROPERTY(bool  isReversed          READ isReversed         NOTIFY isReversedChanged)
    Q_PROPERTY(bool  isValueMapped       READ isValueMapped      NOTIFY isValueMappedChanged)

    float inMin            () const;
    float inMax            () const;
    float inRange          () const;
    float outMin           () const;
    float outMax           () const;
    float outRange         () const;
    float rawValue         () const;
    float normalizedValue  () const;
    float mappedValue      () const;
    bool  IsClamping () const;
    bool  isReversed       () const;
    bool  isValueMapped    () const;

    Q_INVOKABLE void setInMin  (float value);
    Q_INVOKABLE void setInMax  (float value);
    Q_INVOKABLE void setOutMin (float value);
    Q_INVOKABLE void setOutMax (float value);

    Q_INVOKABLE void setRanges   (float inMin, float inMax, float outMin, float outMax);
    Q_INVOKABLE void setInRange  (float min, float max);
    Q_INVOKABLE void setOutRange (float min, float max);

    Q_INVOKABLE void setIsClamping (bool state);
    Q_INVOKABLE void setIsReversed       (bool state);

    void invalidate  ();
    void resetValue  ();
    void remapValue  ();
    void setRawValue (float value);
    
    bool isInInputRange   (float value);
    bool isInOutputRange  (float value);
    void expandInputToFit (float value);
    
private:
    float _inMin            = 0.0f;
    float _inMax            = 1.0f;
    float _inRange          = 1.0f;
    float _outMin           = 1000.0f;
    float _outMax           = 2000.0f;
    float _outRange         = 1000.0f;
    float _rawValue         = 0.0f;
    float _normalizedValue  = 0.0f;
    float _mappedValue      = 1000.0f;
    bool  _IsClamping = true;
    bool  _isReversed       = false;
    bool  _isValueMapped    = false;

signals:
    void inMinChanged();
    void inMaxChanged();
    void inRangeChanged();
    void outMinChanged();
    void outMaxChanged();
    void outRangeChanged();
    void rawValueChanged();
    void normalizedValueChanged();
    void mappedValueChanged();
    void IsClampingChanged();
    void isReversedChanged();
    void isValueMappedChanged();
};

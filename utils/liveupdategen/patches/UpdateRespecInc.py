#@liveupdate("globalClassMethod", "form.attributeRespecWindow::AttributeRespecWindow", "IncreaseAttribute")
def IncreaseAttribute(self, attribute, *args):
    if self.respecBar[attribute].GetValue() >= 20:
        return
    if self.unspentPts <= 0:
        raise UserError('RespecCannotIncrementNotEnoughPoints')
    if not self.respecBar[attribute].Increment():
        raise UserError('RespecAttributesTooHigh')

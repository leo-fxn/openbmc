# libcereal needs 'serialization' when ptest is enabled.
# This will be set upstream with Ie5fa23a104772568cb25500f2cb5af20bc1e2402. 
BOOST_LIBS:openbmc-phosphor:class-target:append = " serialization"

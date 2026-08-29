# Verification plan

- Unit test pure safety and conversion logic on the host.
- Test drivers using sensor mocks and recorded input vectors.
- Check task stack high-water marks and runtime statistics.
- Measure control-loop period, jitter and deadline misses with GPIO tracing.
- Inject stale commands, low battery, overtemperature, sensor failure and link loss.
- Confirm every critical fault stops motors and reports a diagnostic code.
- Use simulation before power-stage and actuator tests.

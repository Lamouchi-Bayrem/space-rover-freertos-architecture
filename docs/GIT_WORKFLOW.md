# Git workflow

```bash
git checkout -b feature/imu-driver
git add firmware/drivers
git commit -m "feat(imu): add timestamped sampling interface"
git push -u origin feature/imu-driver
```

Use small commits and GitHub Issues. Suggested labels: `firmware`, `freertos`, `ros2`,
`simulation`, `safety`, `test`, `documentation`, and `hardware`.

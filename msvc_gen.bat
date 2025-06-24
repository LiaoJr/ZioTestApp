@if exist vs_build ( rmdir /q /s vs_build )
@mkdir vs_build
@cd vs_build

@rem 设置VS生成器
@set VS_GENERATOR="Visual Studio 14 Win64"

cmake -G %VS_GENERATOR% ../
pause
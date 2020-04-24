% execute this mex file on Linux in Matlab once. See also Cmake driven
% build if this fails

fsep = '/';

pathcopyFrom = sprintf(['..' fsep '..' fsep 'Core' fsep 'regularisers_CPU'], 1i);
pathcopyFrom1 = sprintf(['..' fsep '..' fsep 'Core' fsep 'CCPiDefines.h'], 1i);
pathcopyFrom2 = sprintf(['..' fsep '..' fsep 'Core' fsep 'inpainters_CPU'], 1i);

copyfile(pathcopyFrom, 'regularisers_CPU');
copyfile(pathcopyFrom1, 'regularisers_CPU');
copyfile(pathcopyFrom2, 'regularisers_CPU');

cd regularisers_CPU
%%
Pathmove = sprintf(['..' fsep 'installed' fsep], 1i);

fprintf('%s \n', '<<<<<<<<<<<Compiling CPU regularisers>>>>>>>>>>>>>');

fprintf('%s \n', 'Compiling ROF-TV...');
mex ROF_TV.c ROF_TV_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('ROF_TV.mex*',Pathmove);

fprintf('%s \n', 'Compiling FGP-TV...');
mex FGP_TV.c FGP_TV_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('FGP_TV.mex*',Pathmove);

fprintf('%s \n', 'Compiling SB-TV...');
mex SB_TV.c SB_TV_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('SB_TV.mex*',Pathmove);

fprintf('%s \n', 'Compiling PD-TV...');
mex PD_TV.c PD_TV_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('PD_TV.mex*',Pathmove);
 
fprintf('%s \n', 'Compiling dFGP-TV...');
mex FGP_dTV.c FGP_dTV_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('FGP_dTV.mex*',Pathmove);
 
fprintf('%s \n', 'Compiling TNV...');
mex TNV.c TNV_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('TNV.mex*',Pathmove);
 
fprintf('%s \n', 'Compiling NonLinear Diffusion...');
mex NonlDiff.c Diffusion_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('NonlDiff.mex*',Pathmove);

fprintf('%s \n', 'Compiling Anisotropic diffusion of higher order...');
mex Diffusion_4thO.c Diffus4th_order_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('Diffusion_4thO.mex*',Pathmove);

fprintf('%s \n', 'Compiling TGV...');
mex TGV.c TGV_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('TGV.mex*',Pathmove);
 
fprintf('%s \n', 'Compiling ROF-LLT...');
mex LLT_ROF.c LLT_ROF_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('LLT_ROF.mex*',Pathmove);
 
fprintf('%s \n', 'Compiling NonLocal-TV...');
mex PatchSelect.c PatchSelect_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
mex Nonlocal_TV.c Nonlocal_TV_core.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('Nonlocal_TV.mex*',Pathmove);
movefile('PatchSelect.mex*',Pathmove);

fprintf('%s \n', 'Compiling additional tools...');
mex TV_energy.c utils.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
movefile('TV_energy.mex*',Pathmove);
 
delete SB_TV_core* ROF_TV_core* FGP_TV_core* FGP_dTV_core* TNV_core* utils* Diffusion_core* Diffus4th_order_core* TGV_core* LLT_ROF_core* CCPiDefines.h
delete PatchSelect_core* Nonlocal_TV_core*
delete PD_TV_core*
fprintf('%s \n', '<<<<<<< CPU regularisers were successfully compiled! >>>>>>>');

pathA2 = sprintf(['..' fsep '..' fsep '..' fsep '..' fsep 'demos' fsep 'Matlab_demos'], 1i);
cd(pathA2);

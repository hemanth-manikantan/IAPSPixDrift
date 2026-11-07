export KMP_AFFINITY=disabled
export OMP_NUM_THREADS=4
cd /home/admhemmant/Softwares/Ansys2022R2/ansys_inc/v222/ansys/bin
./ansys222 -b -i /home/admhemmant/Analysis/GridPix/Simulations/IAPSPixDrift/fieldmap_anodeclear_900um/IAPSPix-DriftField.inp \
  -o /home/admhemmant/Analysis/GridPix/Simulations/IAPSPixDrift/fieldmap_anodeclear_900um/test.out \
  -np 1 -dir /home/admhemmant/Analysis/GridPix/Simulations/IAPSPixDrift/fieldmap_anodeclear_900um
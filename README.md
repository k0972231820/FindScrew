<h1>利用OpenCV區分出盒中的螺絲與螺帽</h1>
<br>

FindScrew
![image](https://user-images.githubusercontent.com/8476048/111864181-7a9cd700-899a-11eb-9bec-68a45511a975.png)
<br>
<h2>●找出螺絲</h2>
首先利用GaussianBlur去除影像雜訊

![image](https://user-images.githubusercontent.com/8476048/112145401-4123d000-8c15-11eb-87dd-9d3bd2e460ac.png)


利用threshold找出影像中閥值高的區域(螺絲與螺帽)，並將影像二值化

![image](https://user-images.githubusercontent.com/8476048/112145747-b4c5dd00-8c15-11eb-89aa-af3fd978d3ac.png)


為了減少螺絲與螺帽相連的情況，對影像使用侵蝕(erode)

![image](https://user-images.githubusercontent.com/8476048/112145915-eb035c80-8c15-11eb-94a1-afcd6112148b.png)

接著透過findContours找出所有螺絲與螺帽的輪廓

![image](https://user-images.githubusercontent.com/8476048/112146102-230a9f80-8c16-11eb-9204-c376ee259808.png)


最後算出包含每個輪廓的最小矩形，並透過該矩形的長寬篩選出螺絲的輪廓並記錄

![image](https://user-images.githubusercontent.com/8476048/112146355-71b83980-8c16-11eb-93f8-881ecd96d222.png)


<br><br>

FindNut

![image](https://user-images.githubusercontent.com/8476048/111644682-0697ed00-883b-11eb-88de-b983667f237e.png)
<br>
<h2>●找出螺帽</h2>
利用與螺絲類似的作法，並找出較小的輪廓

![image](https://user-images.githubusercontent.com/8476048/112146554-ad530380-8c16-11eb-896e-17a36ca6e0f5.png)


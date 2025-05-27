import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from matplotlib import rc
# from matplotlib import rcParams

## matplotlib 配置

### 字体
###FIXME: 中文字体不显示
rc('font', family='Adobe Heiti Std', size=12)
# plt.rcParams['font.sans-serif'] = ['SimHei', 'DejaVu Sans']
# plt.rcParams['font.serif'] = ['Times New Roman', 'DejaVu Serif']
# plt.rcParams['axes.unicode_minus'] = False

### 图表样式
sns.set_style("whitegrid")
plt.style.use('seaborn-v0_8')

### 颜色
colors = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#96CEB4', '#FFEAA7']
thread_colors = dict(zip([1, 2, 4, 8, 16], colors))

## 数据处理

files = {
  'data/cpu_threads1.csv': 1,
  'data/cpu_threads2.csv': 2,
  'data/cpu_threads4.csv': 4,
  'data/cpu_threads8.csv': 8,
  'data/cpu_threads16.csv': 16
}

### 读取
data_dict = {}
for file_path, thread_count in files.items():
  try:
    df = pd.read_csv(file_path)
    df['threads'] = thread_count
    data_dict[thread_count] = df
    print(f"成功读取 {file_path}: {len(df)} 行数据")
  except Exception as e:
    print(f"读取文件 {file_path} 时出错: {e}")

###TODO: 数据清洗
###NOTE: 处理掉异常值/离群值或是限制图轴坐标范围，以及将微秒转换为毫秒

### 合并
all_data = pd.concat(data_dict.values(), ignore_index=True)

## 绘制图表
fig = plt.figure(figsize=(20, 15))

### 粒子数-计算时间折线图
plt.subplot(2, 2, 1)
for thread_count in sorted(data_dict.keys()):
  df = data_dict[thread_count]
  plt.plot(df['object_counts'], df['physics_update_elapsed_time'], 
           color=thread_colors[thread_count], linewidth=2, alpha=0.8,
           label=f'{thread_count} 线程')

plt.xlabel('生成粒子数', fontsize=12, weight='bold')
plt.ylabel('单次计算时间 (μs)', fontsize=12, weight='bold')
plt.title('不同线程数下粒子数与计算时间的关系', fontsize=14, weight='bold')
plt.legend(fontsize=10)
plt.grid(True, alpha=0.3)

### 粒子数-渲染时间折线图
plt.subplot(2, 2, 2)
for thread_count in sorted(data_dict.keys()):
  df = data_dict[thread_count]
  plt.plot(df['object_counts'], df['render_elapsed_time'], 
           color=thread_colors[thread_count], linewidth=2, alpha=0.8,
           label=f'{thread_count} 线程')

plt.xlabel('生成粒子数', fontsize=12, weight='bold')
plt.ylabel('单次渲染时间 (μs)', fontsize=12, weight='bold')
plt.title('不同线程数下粒子数与渲染时间的关系', fontsize=14, weight='bold')
plt.legend(fontsize=10)
plt.grid(True, alpha=0.3)

### 线程数-计算时间箱线图
plt.subplot(2, 2, 3)
physics_data = []
thread_labels = []
for thread_count in sorted(data_dict.keys()):
  physics_data.append(data_dict[thread_count]['physics_update_elapsed_time'])
  thread_labels.append(f'{thread_count}线程')

box_plot1 = plt.boxplot(physics_data, labels=thread_labels, patch_artist=True)
for patch, color in zip(box_plot1['boxes'], colors):
  patch.set_facecolor(color)
  patch.set_alpha(0.7)

plt.xlabel('线程数', fontsize=12, weight='bold')
plt.ylabel('单次计算时间 (μs)', fontsize=12, weight='bold')
plt.title('不同线程数的计算时间分布', fontsize=14, weight='bold')
plt.grid(True, alpha=0.3)

### 线程数-渲染时间箱线图
plt.subplot(2, 2, 4)
render_data = []
for thread_count in sorted(data_dict.keys()):
  render_data.append(data_dict[thread_count]['render_elapsed_time'])

box_plot2 = plt.boxplot(render_data, labels=thread_labels, patch_artist=True)
for patch, color in zip(box_plot2['boxes'], colors):
  patch.set_facecolor(color)
  patch.set_alpha(0.7)

plt.xlabel('线程数', fontsize=12, weight='bold')
plt.ylabel('单次渲染时间 (μs)', fontsize=12, weight='bold')
plt.title('不同线程数的渲染时间分布', fontsize=14, weight='bold')
plt.grid(True, alpha=0.3)

## 导出与打印
##TODO: 将子图改成单独的图表
plt.tight_layout(pad=3.0)
plt.savefig('data/cpu_threads_analysis.png', dpi=300, bbox_inches='tight')
plt.show()

print("\n=== 数据统计摘要 ===")
for thread_count in sorted(data_dict.keys()):
  df = data_dict[thread_count]
  print(f"\n{thread_count} 线程:")
  print(f"  计算时间 - 平均: {df['physics_update_elapsed_time'].mean():.2f}μs, "
        f"标准差: {df['physics_update_elapsed_time'].std():.2f}μs")
  print(f"  渲染时间 - 平均: {df['render_elapsed_time'].mean():.2f}μs, "
        f"标准差: {df['render_elapsed_time'].std():.2f}μs")

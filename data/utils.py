import pandas as pd
import matplotlib.pyplot as plt

font = {
  'family': ['Times New Roman', 'SimHei', 'DejaVu Sans'],
  'size': 12,
  'weight': 'bold'
}

colors5 = ['#414861', '#615541', '#E09825', '#1C92A7', '#264DE0']
colors6 = ['#31E041', '#E08631', '#8B6B4D', '#466148', '#4D4661', '#6031E0']

def smooth_data(df, columns, window_size=10):
  """
  使用移动平均对数据平滑处理
  """
  df_smooth = df.copy()
  
  for col in columns:
    df_smooth[f'{col}_smooth'] = df_smooth[col].rolling(window=window_size, center=True).mean()
  
  return df_smooth

def remove_outliers(df, columns):
  """
  使用IQR方法移除离群值
  """
  df_clean = df.copy()
  for col in columns:
    Q1 = df_clean[col].quantile(0.25)
    Q3 = df_clean[col].quantile(0.75)
    IQR = Q3 - Q1
    lower_bound = Q1 - 1.5 * IQR
    upper_bound = Q3 + 1 * IQR
    df_clean = df_clean[(df_clean[col] >= lower_bound) & (df_clean[col] <= upper_bound)]
  return df_clean

def calculate_total_physics_time(df, max_objects=30000):
  """
  计算前max_objects个粒子对象的总物理计算时间
  """
  filtered_df = df[df['object_counts'] <= max_objects].copy()
  filtered_df['physics_time_ms'] = filtered_df['physics_update_elapsed_time'] / 1000
  total_time = filtered_df['physics_time_ms'].sum()
  
  return total_time, len(filtered_df)